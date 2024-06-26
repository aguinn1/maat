#
# Copyright 2023 United States Government
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

DIST_SUBDIRS =
CLEANFILES=*.xml test-info/*

include ../common.mk.am

if HAVE_DOXYGEN
CLEANFILES += doxygen.log
endif

apb_PROGRAMS=

# APBs shouldn't generally need to be installed setuid.
# However, "fat" APBs that implement measurement functionality directly
# may need to be run as root. So we'll set up the necessary machinery.
suid_apb_PROGRAMS=

# APBs that follow the convention of info file named as "<apbname>.xml"
# will automatically have their info file added to the target below.
#
# If for some reason you really must break this convention, you should
# add the info file to this target manually in your if BUILD_my_apb block.
#
apbinfo_DATA=

EXTRA_DIST = $(srcdir)/*.xml.in

AM_CPPFLAGS = $(CPPFLAGS) -I$(top_srcdir)/src/include -I$(srcdir) \
		-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/types \
		$(LIBMAAT_CFLAGS) \
		-DDEFAULT_APB_DIR="\"$(APB_INFO_DIR)\"" \
		-DDEFAULT_ASP_DIR="\"$(ASP_INFO_DIR)\"" \
		-DDEFAULT_MEAS_SPEC_DIR="\"$(SPEC_INSTALL_DIR)\"" \
		-DDEFAULT_MAAT_MONGO_LOC="\"mongodb://localhost:27017/\"" \
		-DDEFAULT_MONGO_DB="\"maat\""

AM_LIBADD = ../types/libmaat_basetypes.la \
		../measurement_spec/libmeasurement_spec.la \
		$(LIBMAAT_APB_LIBS)

APB_COMMON_SOURCES = apb-common.h apb-common.c

if BUILD_COVERAGE
AM_CPPFLAGS += --coverage
AM_LIBADD += -lgcov
endif

if BUILD_process_measurement_APB
apb_PROGRAMS                   += process_measurement_apb
process_measurement_apb_SOURCES	= process_measurement_apb.c $(APB_COMMON_SOURCES)
process_measurement_apb_LDADD	= $(AM_LIBADD)
endif

if BUILD_userspace_APB
apb_PROGRAMS                   += userspace_apb
userspace_apb_SOURCES		= userspace_apb.c userspace_common_funcs.c userspace_common_funcs.h $(APB_COMMON_SOURCES)
userspace_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_layered_att_APB
apb_PROGRAMS                   += layered_att_apb
layered_att_apb_SOURCES		= layered_att_apb.c userspace_common_funcs.c userspace_common_funcs.h userspace_appraiser_common_funcs.c userspace_appraiser_common_funcs.h $(APB_COMMON_SOURCES)
layered_att_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_complex_att_APB
apb_PROGRAMS                   += complex_att_apb
complex_att_apb_SOURCES		= complex_att_apb.c userspace_common_funcs.c userspace_common_funcs.h $(APB_COMMON_SOURCES)
complex_att_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_passport_ret_APB
apb_PROGRAMS                   += passport_ret_apb
passport_ret_apb_SOURCES	= passport_ret_apb.c $(APB_COMMON_SOURCES)
passport_ret_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_passport_appraiser_APB
AM_CPPFLAGS		       += $(json-c_CFLAGS) $(JSON_CLFAGS) $(LIBMONGOC_CFLAGS) $(LIBBSON_CFLAGS)
apb_PROGRAMS                   += passport_appraiser_apb
apbinfo_DATA                   += datafiles/passport-config.txt
passport_appraiser_apb_SOURCES	= passport_appraiser_apb.c $(APB_COMMON_SOURCES) 
passport_appraiser_apb_LDADD	= $(json-c_LIBS) $(JSON_LIBS) $(AM_LIBADD) $(LIBMAAT_CLIENT_LIBS) $(LIBMAAT_AM_LIBS) \
				  $(LIBMONGOC_LIBS) $(LIBSON_LIBS)
endif

if BUILD_passport_storage_APB
apb_PROGRAMS                   += passport_storage_apb
passport_storage_apb_SOURCES	= passport_storage_apb.c $(APB_COMMON_SOURCES)
passport_storage_apb_LDADD	= $(AM_LIBADD)
endif

if BUILD_request_passport_APB
apb_PROGRAMS                   += request_passport_apb
request_passport_apb_SOURCES	= request_passport_apb.c $(APB_COMMON_SOURCES)
request_passport_apb_LDADD	= $(AM_LIBADD)
endif

if BUILD_passport_userspace_appraiser_APB
apb_PROGRAMS				 += passport_userspace_appraiser_apb
passport_userspace_appraiser_apb_SOURCES  = passport_userspace_appraiser_apb.c $(APB_COMMON_SOURCES)
passport_userspace_appraiser_apb_LDADD    = $(AM_LIBADD) $(LIBMAAT_CLIENT_LIBS) $(LIBMAAT_AM_LIBS)
endif

if BUILD_quiot_tlm_APB
apb_PROGRAMS                   += quiot_tlm_apb
quiot_tlm_apb_SOURCES		= quiot_tlm_apb.c $(APB_COMMON_SOURCES)
quiot_tlm_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_quiot_tlm_appraiser_APB
apb_PROGRAMS                   += quiot_tlm_appraiser_apb
quiot_tlm_appraiser_apb_SOURCES	= quiot_tlm_appraiser_apb.c $(APB_COMMON_SOURCES)
quiot_tlm_appraiser_apb_LDADD	= $(AM_LIBADD)
endif

if BUILD_appraiser_APB
apb_PROGRAMS                   += appraiser_apb
appraiser_apb_SOURCES		= appraiser_apb.c  $(APB_COMMON_SOURCES)
appraiser_apb_LDADD		= $(AM_LIBADD) \
				  $(LIBMAAT_CLIENT_LIBS) $(LIBMAAT_AM_LIBS)
endif

if BUILD_hashdir_APB
apb_PROGRAMS                   += hashdir_apb
hashdir_apb_SOURCES		= hashdir_apb.c  $(APB_COMMON_SOURCES)
hashdir_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_hashfile_APB
apb_PROGRAMS                   += hashfile_apb
hashfile_apb_SOURCES		= hashfile_apb.c  $(APB_COMMON_SOURCES)
hashfile_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_kim_APB
apb_PROGRAMS                   += kim_apb
kim_apb_SOURCES		= kim_apb.c  $(APB_COMMON_SOURCES)
kim_apb_LDADD			= $(AM_LIBADD)
endif

if BUILD_iot_uart_APB
apb_PROGRAMS                   += iot_uart_apb
iot_uart_apb_SOURCES		= iot_uart_apb.c  $(APB_COMMON_SOURCES)
iot_uart_apb_LDADD		= $(AM_LIBADD)
endif

if BUILD_iot_appraiser_APB
apb_PROGRAMS                   += iot_appraiser_apb
iot_appraiser_apb_SOURCES	= iot_appraiser_apb.c  $(APB_COMMON_SOURCES)
iot_appraiser_apb_LDADD	= $(AM_LIBADD)
endif

if BUILD_userspace_appraiser_APB
apb_PROGRAMS			+= userspace_appraiser_apb
userspace_appraiser_apb_SOURCES  = userspace_appraiser_apb.c userspace_appraiser_common_funcs.c userspace_appraiser_common_funcs.h $(APB_COMMON_SOURCES)
userspace_appraiser_apb_LDADD    = $(AM_LIBADD)
endif

if BUILD_layered_appraiser_APB
apb_PROGRAMS			+= layered_appraiser_apb
layered_appraiser_apb_SOURCES  = layered_appraiser_apb.c userspace_appraiser_common_funcs.c userspace_appraiser_common_funcs.h $(APB_COMMON_SOURCES)
layered_appraiser_apb_LDADD    = $(AM_LIBADD)
endif

if BUILD_forwarding_APB
apb_PROGRAMS			+= forwarding_apb
forwarding_apb_SOURCES = forwarding_apb.c
forwarding_apb_LDADD   = $(AM_LIBADD) $(LIBMAAT_CLIENT_LIBS) $(LIBMAAT_AM_LIBS)
endif

if BUILD_no_op_APB
apb_PROGRAMS			+= no_op_apb
no_op_apb_SOURCES = no_op_apb.c
no_op_apb_LDADD   = $(AM_LIBADD) $(LIBMAAT_CLIENT_LIBS) $(LIBMAAT_AM_LIBS)
endif

if BUILD_deleg_meas_skeleton_APB
apb_PROGRAMS			+= deleg_meas_skeleton_apb
deleg_meas_skeleton_apb_SOURCES = deleg_meas_skeleton_apb.c $(APB_COMMON_SOURCES)
deleg_meas_skeleton_apb_LDADD   = $(AM_LIBADD)
endif

if BUILD_deleg_meas_appraise_skeleton_APB
apb_PROGRAMS			+= deleg_meas_appraise_skeleton_apb
deleg_meas_appraise_skeleton_apb_SOURCES = deleg_meas_appraise_skeleton_apb.c $(APB_COMMON_SOURCES)
deleg_meas_appraise_skeleton_apb_LDADD   = $(AM_LIBADD)
endif

apbinfo_DATA+=$(patsubst %, %.xml, $(apb_PROGRAMS) $(suid_apb_PROGRAMS))
check_DATA=$(patsubst %.xml, test-info/%.xml, $(apbinfo_DATA))

install-data-hook:
	$(foreach apb, $(suid_apb_PROGRAMS), chmod u+s $(DESTDIR)/$(apbdir)/$(apb);)

docs:
if HAVE_DOXYGEN
	$(AM_V_GEN)$(DOXYGEN) $(builddir)/Doxyfile
endif

distclean-local: clean-local

clean-local:
	rm -rf html
	rm -rf xml
	rm -rf latex
