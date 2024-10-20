/*
 * Copyright 2023 United States Government
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/*! \file
 * This ASP reads the measurement contract from fd_in,
 * verifies its signatures and basic structure, and
 * writes the verification result ("PASS" or "FAIL")
 * to fd_out
 *
 * Usage: "ASP_NAME" <fd_in> <fd_out> <workdir> <nonce> <cacert> <akpubkey> <verify_tpm>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <fcntl.h>

#include <util/util.h>
#include <util/xml_util.h>
#include <util/signfile.h>
#include <util/base64.h>

#include <asp/asp-api.h>
#include <measurement_spec/find_types.h>
#include <common/asp-errno.h>
#include <util/maat-io.h>

#include <maat-basetypes.h>
#include <sys/types.h>
#include <client/maat-client.h>

#define ASP_NAME "verify_measurement_contract_asp"

#define MAX_RECV_BUF_SZ INT_MAX
#define TIMEOUT 1000

/**
 * @brief Verify the signature and nonce within the measurement contract
 *
 * @param workdir A string containing the path of the working directory of the
 *        Attestation Manager
 * @param nonce A string containg this attestation scenario's nonce
 * @param cacert A string containing the file path of the CA certificate that can be
 *        used to verify the signatures in the contract
 * @param akpubkey A string containing the file path the AK public key generated by the TPM
 * @param verify_tpm An integer containing a 1 or a 0 to indicate if TPM-based signature
 *        verification should be employed or not (respectively)
 * @param buf A buffer which contains the serialized contract XML
 * @param buf_size The size of buf
 *
 * @return int Returns 0 on success, < 0 on error
 */
static int verify_contract(char *workdir, char *nonce, char *cacert,
                           char *akpubkey, int verify_tpm, void *buf, size_t buf_size)
{
    int ret = -1;
    int i = 0;

    xmlDoc *doc             = NULL;
    xmlNode *root           = NULL;
    xmlXPathObject *subcobj = NULL;

    char tmpstr[200]        = {0};
    char *contract_type     = NULL;

    /* Cast is acceptable because the buffer contains the bytes of a serialized XML document representing
     * a measurement contract, and representing this as a char * is valid */
    doc = get_doc_from_blob((char *)buf, buf_size);
    if (doc == NULL) {
        dlog(0, "Failed to parse contract XML.\n");
        goto xml_err;
    }

    root = xmlDocGetRootElement(doc);
    if (root == NULL) {
        dlog(1, "Failed to get contract root node.\n");
        goto root_err;
    }

    contract_type = xmlGetPropASCII(root, "type");
    if(contract_type == NULL) {
        dlog(1, "Failed to get contract type attribute.\n");
        goto contract_type_read_err;
    }

    if (strcasecmp(contract_type, "measurement") != 0) {
        dlog(1, "Not a measurement contract\n");
        goto contract_type_value_err;
    }

    subcobj = xpath(doc, "/contract/subcontract");
    if(subcobj == NULL) {
        dlog(1, "obj from xpath is null\n");
        goto subcontract_read_err;
    }

    if (!subcobj->nodesetval) {
        dlog(1, "No subcontracts?\n");
        goto subcontract_err;
    }

    snprintf(tmpstr, 200, "%s/cred", workdir);

    for (i=0; i<subcobj->nodesetval->nodeNr; i++) {
        if (subcobj->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE) {
            if (verify_tpm) {
                ret = verify_xml(doc,
                                 subcobj->nodesetval->nodeTab[i],
                                 tmpstr,
                                 nonce,
                                 akpubkey,
                                 SIGNATURE_TPM,
                                 cacert);
            } else {
                ret = verify_xml(doc,
                                 subcobj->nodesetval->nodeTab[i],
                                 tmpstr,
                                 nonce,
                                 akpubkey,
                                 SIGNATURE_OPENSSL,
                                 cacert);
            }

            if (ret != MAAT_SIGNVFY_SUCCESS) {
                /* bad signature? */
                dlog(0, "Signature for subcontract %d failed\n", i);
                goto subcontract_sig_err;
            }
        }
    }

    ret = 0;

subcontract_sig_err:
subcontract_err:
subcontract_read_err:
    xmlXPathFreeObject(subcobj);
contract_type_value_err:
contract_type_read_err:
    free(contract_type);
root_err:
    xmlFreeDoc(doc);
xml_err:
    return ret;

}

int asp_init(int argc UNUSED, char *argv[] UNUSED)
{
    int ret_val = 0;
    asp_loginfo("Initialized verify measurement_contract ASP\n");

    if((ret_val = register_types()) < 0) {
        return ret_val;
    }

    asp_logdebug("verify_measurement_contract asp done init (success)\n");

    return ASP_APB_SUCCESS;
}

int asp_exit(int status)
{
    asp_loginfo("Exiting verify_measurement_contract ASP\n");
    return status;
}

int asp_measure(int argc, char *argv[])
{
    dlog(6, "IN verify_measurement_contract ASP MEASURE\n");

    // Cmd line args
    int fd_in            = -1;
    int fd_out           = -1;
    char *workdir        = NULL;
    char *nonce          = NULL;
    char *cacert         = NULL;
    char *akpubkey       = NULL;
    int verify_tpm       = -1;

    // Parse variables
    long int parse_fd_in;
    long int parse_fd_out;
    long int parse_verify_tpm;

    // Buf in
    unsigned char *buf   = NULL;
    size_t bufsize       = 0;

    // Buf out
    char *out            = NULL;
    size_t outsize       = 0;

    // IO status values
    size_t bytes_written = 0;
    size_t bytes_read    = 0;
    int eof_enc          = 0;

    // Return value
    int ret_val          = 0;

    errno = 0;

    if((argc < 7) ||
            (((parse_fd_in = strtol(argv[1], NULL, 10)) < 0) || errno != 0) ||
            (((parse_fd_out = strtol(argv[2], NULL, 10)) < 0) || errno != 0) ||
            ((workdir    = argv[3]) == NULL) ||
            ((nonce      = argv[4]) == NULL) ||
            ((cacert    = argv[5]) == NULL)  ||
            ((akpubkey    = argv[6]) == NULL)  ||
            (((parse_verify_tpm = strtol(argv[7], NULL, 10)) < 0) || errno != 0)) {
        asp_logerror("Usage: "ASP_NAME" <fd_in> <fd_out> <workdir> <nonce> <cacert> <akpubkey> <verify_tpm>\n");
        ret_val = -EINVAL;
        goto parse_args_failed;
    }

    if(parse_fd_in > INT_MAX || parse_fd_in < INT_MIN) {
        asp_logerror("Input file descriptor value %ld too large for bounds of type\n", parse_fd_in);
        ret_val = -EINVAL;
        goto parse_args_failed;
    }

    fd_in = (int)parse_fd_in;

    if(parse_fd_out > INT_MAX || parse_fd_out < INT_MIN) {
        asp_logerror("Input file descriptor value %ld too large for bounds of type\n", parse_fd_out);
        ret_val = -EINVAL;
        goto parse_args_failed;
    }

    fd_out = (int)parse_fd_out;

    if(parse_verify_tpm > INT_MAX || parse_verify_tpm < INT_MIN) {
        asp_logerror("Input file descriptor value %ld too large for bounds of type\n", parse_verify_tpm);
        ret_val = -EINVAL;
        goto parse_args_failed;
    }

    verify_tpm = (int)parse_verify_tpm;

    if(!(verify_tpm == 0 || verify_tpm == 1)) {
        asp_logerror("Usage: "ASP_NAME" <fd_in> <fd_out> <workdir> <nonce> <cacert> <akpubkey> <verify_tpm>\n");
        ret_val = -EINVAL;
        goto parse_args_failed;
    }

    // Read the measurement contract
    ret_val = maat_read_sz_buf(fd_in, &buf, &bufsize, &bytes_read, &eof_enc,
                               TIMEOUT, MAX_RECV_BUF_SZ);
    if(ret_val < 0 && ret_val != -EAGAIN) {
        dlog(0, "Error reading evidence from channel\n");
        ret_val = -1;
        goto read_failed;
    } else if (ret_val == -EAGAIN) {
        dlog(2, "Warning: timeout occured before read could complete\n");
    } else if (eof_enc != 0) {
        dlog(0, "Error: EOF encountered before complete buffer read\n");
        ret_val = -1;
        goto eof_enc;
    }

    // Verify the measurement contract
    ret_val = verify_contract(workdir, nonce, cacert, akpubkey, verify_tpm, buf, bufsize);
    if (ret_val == 0) {
        out = "PASS";
        dlog(4, "Contract passed verification\n");
        outsize = 5;
    } else {
        out = "FAIL";
        dlog(4, "Contract failed verification\n");
        outsize = 5;
    }

    // Write the verification result to fd_out
    // Cast is justified because the function does not regard the signedness of the argument
    ret_val = maat_write_sz_buf(fd_out, (unsigned char *)out, outsize, &bytes_written, TIMEOUT);
    if(ret_val < 0) {
        dlog(0, "Error writing verification result to channel\n");
        ret_val = -1;
        goto write_failed;
    } else if (ret_val == EAGAIN) {
        dlog(4, "Warning: timeout occured before write could complete\n");
    }

    dlog(6, "buffer size: %zu, bytes_written: %zu\n", outsize, bytes_written);

    ret_val = ASP_APB_SUCCESS;
    asp_loginfo("verify_measurement_contract ASP returning with success\n");

write_failed:
eof_enc:
    free(buf);
    bufsize = 0;
read_failed:
    close(fd_out);
    close(fd_in);
parse_args_failed:
    return ret_val;
}
