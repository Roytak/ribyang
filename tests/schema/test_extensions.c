/**
 * \file test_extensions.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - extensions
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/files"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"

struct state {
    struct ly_ctx *ctx;
    int fd;
    char *str1;
    char *str2;
};

static int
setup_ctx(void **state, const char *searchdir)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(searchdir);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    st->fd = -1;

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
setup_ctx_yin(void **state)
{
    return setup_ctx(state, SCHEMA_FOLDER_YIN);
}

static int
setup_ctx_yang(void **state)
{
    return setup_ctx(state, SCHEMA_FOLDER_YANG);
}

static int
teardown_ctx(void **state)
{
    struct state *st = (*state);

    ly_ctx_destroy(st->ctx, NULL);
    if (st->fd >= 0) {
        close(st->fd);
    }
    free(st->str1);
    free(st->str2);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_fullset_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    struct stat s;

    mod = lys_parse_path(st->ctx, SCHEMA_FOLDER_YIN"/ext.yin", LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);

    st->fd = open(SCHEMA_FOLDER_YIN"/ext.yin", O_RDONLY);
    fstat(st->fd, &s);
    st->str2 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str2, NULL);
    assert_int_equal(read(st->fd, st->str2, s.st_size), s.st_size);
    st->str2[s.st_size] = '\0';

    assert_string_equal(st->str1, st->str2);
}

static void
test_module_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </yang-version>\n  <namespace uri=\"urn:ext\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </namespace>\n  <prefix value=\"x\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </prefix>\n"
                    "  <import module=\"ext-def\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <prefix value=\"e\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </prefix>\n    <revision-date date=\"2016-01-18\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </revision-date>\n    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </import>\n"
                    "  <include module=\"ext-inc\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <revision-date date=\"2016-01-18\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </revision-date>\n    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </include>\n"
                    "  <revision date=\"2016-01-20\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </revision>\n  <revision date=\"2016-01-18\">\n"
                    "    <e:a/>\n"
                    "  </revision>\n"
                    "  <e:a/>\n  <e:b x=\"one\"/>\n  <e:c>\n    <e:y>one</e:y>\n  </e:c>\n"
                    "</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_module_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1 {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n  namespace \"urn:ext\" {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n  prefix x {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n\n"
                    "  import ext-def {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    prefix e {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    revision-date 2016-01-18 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  include ext-inc {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    revision-date 2016-01-18 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  revision \"2016-01-20\" {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n"
                    "  revision \"2016-01-18\" {\n"
                    "    e:a;\n"
                    "  }\n\n"
                    "  e:a;\n  e:b \"one\";\n  e:c \"one\";\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_container_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <container name=\"c\">\n"
                    "    <presence value=\"test\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </presence>\n"
                    "    <config value=\"false\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </container>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_container_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  namespace \"urn:ext\";\n  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  container c {\n    presence \"test\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    config false {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_leaf_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <typedef name=\"length\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <type name=\"int32\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </type>\n"
                    "    <units name=\"meter\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </units>\n"
                    "    <default value=\"10\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </typedef>\n"
                    "  <leaf name=\"l\">\n"
                    "    <type name=\"string\">\n"
                    "      <pattern value=\"[a-z]\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <modifier value=\"invert-match\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </modifier>\n        <error-message>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value>emsg</value>\n"
                    "        </error-message>\n        <error-app-tag value=\"eapptag\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </error-app-tag>\n        <description>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>desc</text>\n"
                    "        </description>\n        <reference>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>ref</text>\n"
                    "        </reference>\n"
                    "      </pattern>\n"
                    "    </type>\n"
                    "    <units name=\"petipivo\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </units>\n"
                    "    <must condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <error-message>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <value>emsg</value>\n"
                    "      </error-message>\n      <error-app-tag value=\"eapptag\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </error-app-tag>\n      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </must>\n"
                    "    <config value=\"false\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <mandatory value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </mandatory>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </leaf>\n  <leaf name=\"d\">\n"
                    "    <type name=\"length\"/>\n"
                    "    <default value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "  </leaf>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_leaf_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  typedef length {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    type int32 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    units \"meter\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    default \"10\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  leaf l {\n    type string {\n"
                    "      pattern \"[a-z]\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        modifier invert-match {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-message\n          \"emsg\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-app-tag \"eapptag\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        description\n          \"desc\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        reference\n          \"ref\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n    }\n"
                    "    units \"petipivo\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    must \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      error-message\n        \"emsg\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      error-app-tag \"eapptag\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    config false {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    mandatory true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n"
                    "  leaf d {\n"
                    "    type int8;\n      default 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_leaflist_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <feature name=\"f1\"/>\n  <feature name=\"f\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <if-feature name=\"f1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </if-feature>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </feature>\n"
                    "  <leaf-list name=\"l1\">\n"
                    "    <when condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </when>\n    <if-feature name=\"f\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </if-feature>\n    <type name=\"string\">\n"
                    "      <length value=\"5\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <error-message>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value>emsg</value>\n"
                    "        </error-message>\n        <error-app-tag value=\"eapptag\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </error-app-tag>\n        <description>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>desc</text>\n"
                    "        </description>\n        <reference>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>ref</text>\n"
                    "        </reference>\n"
                    "      </length>\n"
                    "    </type>\n"
                    "    <units name=\"petipivo\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </units>\n"
                    "    <must condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <error-message>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <value>emsg</value>\n"
                    "      </error-message>\n      <error-app-tag value=\"eapptag\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </error-app-tag>\n      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </must>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <min-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </min-elements>\n"
                    "    <max-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </max-elements>\n"
                    "    <ordered-by value=\"user\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </ordered-by>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </leaf-list>\n"
                    "  <leaf-list name=\"l2\">\n"
                    "    <type name=\"int8\">\n"
                    "      <range value=\"1..10\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <error-message>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value>emsg</value>\n"
                    "        </error-message>\n        <error-app-tag value=\"eapptag\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </error-app-tag>\n        <description>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>desc</text>\n"
                    "        </description>\n        <reference>\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <text>ref</text>\n"
                    "        </reference>\n"
                    "      </range>\n"
                    "    </type>\n"
                    "    <default value=\"1\"/>\n    <default value=\"2\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "  </leaf-list>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_leaflist_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  feature f1;\n\n  feature f {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    if-feature \"f1\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  leaf-list l1 {\n"
                    "    when \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    if-feature \"f\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    type string {\n      length \"5\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        error-message\n          \"emsg\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-app-tag \"eapptag\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        description\n          \"desc\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        reference\n          \"ref\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n    }\n"
                    "    units \"petipivo\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    must \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      error-message\n        \"emsg\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      error-app-tag \"eapptag\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    min-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    max-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    ordered-by user {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  leaf-list l2 {\n"
                    "    type int8 {\n      range \"1..10\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "        error-message\n          \"emsg\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        error-app-tag \"eapptag\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        description\n          \"desc\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        reference\n          \"ref\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n    }\n"
                    "    default \"1\";\n"
                    "    default \"2\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_list_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <list name=\"l\">\n"
                    "    <key value=\"id\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </key>\n"
                    "    <unique tag=\"val1\"/>\n    <unique tag=\"val2\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </unique>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <min-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </min-elements>\n"
                    "    <max-elements value=\"1\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </max-elements>\n"
                    "    <ordered-by value=\"user\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </ordered-by>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <leaf name=\"id\">\n"
                    "      <type name=\"instance-identifier\">\n"
                    "        <e:a/>\n"
                    "        <require-instance value=\"true\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </require-instance>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "    <leaf name=\"val1\">\n"
                    "      <type name=\"decimal64\">\n"
                    "        <e:a/>\n"
                    "        <fraction-digits value=\"2\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </fraction-digits>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "    <leaf name=\"val2\">\n"
                    "      <type name=\"leafref\">\n"
                    "        <e:a/>\n"
                    "        <path value=\"../val1\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </path>\n"
                    "        <require-instance value=\"true\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </require-instance>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "  </list>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_list_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  list l {\n"
                    "    key \"id\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    unique \"val1f\";\n    unique \"val2\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    min-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    max-elements 1 {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    ordered-by user {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    leaf id {\n"
                    "      type instance-identifier {\n"
                    "        e:a;\n"
                    "        require-instance true {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n"
                    "    }\n    leaf val1 {\n"
                    "      type decimal64 {\n"
                    "        e:a;\n"
                    "        fraction-digits 2 {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n"
                    "    }\n    leaf val2 {\n"
                    "      type leafref {\n"
                    "        e:a;\n"
                    "        path \"../val1\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n        require-instance true {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_anydata_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <feature name=\"f\"/>\n"
                    "  <anyxml name=\"a\">\n"
                    "    <when condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </when>\n    <if-feature name=\"f\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </if-feature>\n"
                    "    <must condition=\"true()\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <error-message>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <value>emsg</value>\n"
                    "      </error-message>\n      <error-app-tag value=\"eapptag\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </error-app-tag>\n      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </must>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <mandatory value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </mandatory>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </anyxml>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_anydata_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  feature f;\n\n"
                    "  anyxml l {\n"
                    "    when \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    if-feature \"f\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    must \"true()\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      error-message\n        \"emsg\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      error-app-tag \"eapptag\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    mandatory true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_choice_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <choice name=\"ch\">\n"
                    "    <default value=\"a\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </default>\n"
                    "    <config value=\"true\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <mandatory value=\"false\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </mandatory>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <case name=\"a\">\n"
                    "      <status value=\"current\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </status>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "      <leaf name=\"c\">\n"
                    "        <type name=\"bits\">\n"
                    "          <bit name=\"zero\">\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "            <position value=\"0\">\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "            </position>\n"
                    "            <status value=\"current\">\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "            </status>\n"
                    "            <description>\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "              <text>desc</text>\n"
                    "            </description>\n"
                    "            <reference>\n"
                    "              <e:a/>\n              <e:b x=\"one\"/>\n              <e:c>\n                <e:y>one</e:y>\n              </e:c>\n"
                    "              <text>ref</text>\n"
                    "            </reference>\n"
                    "          </bit>\n"
                    "          <bit name=\"one\">\n            <e:a/>\n          </bit>\n"
                    "        </type>\n"
                    "      </leaf>\n"
                    "    </case>\n"
                    "    <leaf name=\"b\">\n"
                    "      <type name=\"enumeration\">\n"
                    "        <enum name=\"one\">\n          <e:a/>\n        </enum>\n"
                    "        <enum name=\"two\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "          <value value=\"2\">\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "          </value>\n"
                    "          <status value=\"current\">\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "          </status>\n"
                    "          <description>\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "            <text>desc</text>\n"
                    "          </description>\n"
                    "          <reference>\n"
                    "            <e:a/>\n            <e:b x=\"one\"/>\n            <e:c>\n              <e:y>one</e:y>\n            </e:c>\n"
                    "            <text>ref</text>\n"
                    "          </reference>\n"
                    "        </enum>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "  </choice>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_choice_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  choice ch {\n"
                    "    default \"a\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    config true {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    mandatory false {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    case a {\n"
                    "      status current {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      leaf c {\n"
                    "        type bits {\n"
                    "          bit zero {\n"
                    "            e:a;\n            e:b \"one\";\n            e:c \"one\";\n"
                    "            position 0 {\n"
                    "              e:a;\n            e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n"
                    "            status current {\n"
                    "              e:a;\n            e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n            description\n              \"desc\" {\n"
                    "              e:a;\n            e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n      reference\n              \"ref\" {\n"
                    "              e:a;\n            e:b \"one\";\n              e:c \"one\";\n"
                    "            }\n          }\n"
                    "          bit one {\n"
                    "            e:a;\n"
                    "          }\n"
                    "      }\n"
                    "    }\n    leaf b {\n"
                    "      type enumeration {\n"
                    "        enum \"one\" {\n"
                    "          e:a;\n"
                    "        }\n"
                    "        enum \"two\" {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "          value 2 {\n"
                    "            e:a;\n            e:b \"one\";\n            e:c \"one\";\n"
                    "          }\n"
                    "          status current {\n"
                    "            e:a;\n            e:b \"one\";\n            e:c \"one\";\n"
                    "          }\n          description\n            \"desc\" {\n"
                    "            e:a;\n            e:b \"one\";\n            e:c \"one\";\n"
                    "          }\n      reference\n            \"ref\" {\n"
                    "            e:a;\n            e:b \"one\";\n            e:c \"one\";\n"
                    "          }\n        }\n      }\n    }\n  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_uses_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <identity name=\"zero\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </identity>\n"
                    "  <identity name=\"one\">\n"
                    "    <base name=\"zero\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </base>\n"
                    "  </identity>\n"
                    "  <identity name=\"two\">\n"
                    "    <base name=\"zero\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </base>\n"
                    "    <base name=\"one\">\n"
                    "      <e:a/>\n"
                    "    </base>\n"
                    "  </identity>\n"
                    "  <grouping name=\"grp\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <container name=\"c\"/>\n"
                    "    <leaf name=\"l\">\n"
                    "      <type name=\"identityref\">\n"
                    "        <base name=\"two\">\n"
                    "          <e:a/>\n          <e:b x=\"one\"/>\n          <e:c>\n            <e:y>one</e:y>\n          </e:c>\n"
                    "        </base>\n"
                    "      </type>\n"
                    "    </leaf>\n"
                    "    <leaf-list name=\"ll1\">\n"
                    "      <type name=\"int8\"/>\n"
                    "    </leaf-list>\n"
                    "    <leaf-list name=\"ll2\">\n"
                    "      <type name=\"int8\"/>\n"
                    "    </leaf-list>\n"
                    "  </grouping>\n"
                    "  <uses name=\"grp\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "    <refine target-node=\"c\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <presence value=\"true\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </presence>\n"
                    "      <config value=\"false\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </config>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "    </refine>\n"
                    "    <refine target-node=\"l\">\n"
                    "      <mandatory value=\"true\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </mandatory>\n"
                    "    </refine>\n"
                    "    <refine target-node=\"ll1\">\n"
                    "      <min-elements value=\"1\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </min-elements>\n"
                    "      <max-elements value=\"1\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </max-elements>\n"
                    "    </refine>\n"
                    "    <refine target-node=\"ll2\">\n"
                    "      <default value=\"1\"/>\n"
                    "      <default value=\"2\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </default>\n"
                    "    </refine>\n"
                    "    <augment target-node=\"c\">\n"
                    "      <status value=\"current\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </status>\n"
                    "      <description>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>desc</text>\n"
                    "      </description>\n"
                    "      <reference>\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "        <text>ref</text>\n"
                    "      </reference>\n"
                    "      <leaf name=\"a\">\n"
                    "        <type name=\"int8\"/>\n"
                    "      </leaf>\n"
                    "    </augment>\n"
                    "  </uses>\n"
                    "</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_uses_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  identity zero {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  identity one {\n"
                    "    base zero {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n\n"
                    "  identity two {\n"
                    "    base zero {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    base one {\n"
                    "      e:a;\n"
                    "    }\n  }\n\n"
                    "  grouping grp {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    container c;\n\n"
                    "    leaf l {\n"
                    "      type identityref {\n"
                    "        base two {\n"
                    "          e:a;\n          e:b \"one\";\n          e:c \"one\";\n"
                    "        }\n      }\n    }\n\n"
                    "    leaf-list ll1 {\n"
                    "      type int8;\n"
                    "    }\n\n"
                    "    leaf-list ll2 {\n"
                    "      type int8;\n"
                    "    }\n  }\n\n"
                    "  uses grp {\n"
                    "    e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    refine \"c\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      presence \"true\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      config false {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    refine \"l\" {\n"
                    "      mandatory \"true\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    refine \"ll1\" {\n"
                    "      min-elements 1 {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      max-elements 1 {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    refine \"ll2\" {\n"
                    "      default \"1\";\n"
                    "      default \"2\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n"
                    "    }\n    augment \"c\" {\n"
                    "      status current {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      description\n        \"desc\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      reference\n        \"ref\" {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n      leaf a {\n"
                    "        type int8;\n"
                    "      }\n    }\n  }\n}\n";


    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_extension_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <yang-version value=\"1.1\"/>\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <extension name=\"x\">\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "    <argument name=\"y\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <yin-element value=\"false\">\n"
                    "        <e:a/>\n        <e:b x=\"one\"/>\n        <e:c>\n          <e:y>one</e:y>\n        </e:c>\n"
                    "      </yin-element>\n"
                    "    </argument>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </extension>\n"
                    "  <e:a>\n"
                    "    <e:a/>\n    <e:b x=\"one\"/>\n    <e:c>\n      <e:y>one</e:y>\n    </e:c>\n"
                    "  </e:a>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_extension_sub_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yang = "module ext {\n"
                    "  yang-version 1.1;\n"
                    "  namespace \"urn:ext\";\n"
                    "  prefix x;\n\n"
                    "  import ext-def {\n    prefix e;\n  }\n\n"
                    "  extension x {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "    argument y {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "      yin-element false {\n"
                    "        e:a;\n        e:b \"one\";\n        e:c \"one\";\n"
                    "      }\n    }\n"
                    "    status current {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    description\n      \"desc\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n    reference\n      \"ref\" {\n"
                    "      e:a;\n      e:b \"one\";\n      e:c \"one\";\n"
                    "    }\n  }\n"
                    "  e:a {\n"
                    "    e:a;\n    e:b \"one\";\n    e:c \"one\";\n"
                    "  }\n}\n";

    mod = lys_parse_mem(st->ctx, yang, LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yang);
}

static void
test_fullset_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    struct stat s;

    mod = lys_parse_path(st->ctx, SCHEMA_FOLDER_YANG"/ext.yang", LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);

    st->fd = open(SCHEMA_FOLDER_YANG"/ext.yang", O_RDONLY);
    fstat(st->fd, &s);
    st->str2 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str2, NULL);
    assert_int_equal(read(st->fd, st->str2, s.st_size), s.st_size);
    st->str2[s.st_size] = '\0';

    assert_string_equal(st->str1, st->str2);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_fullset_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_module_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_container_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leaf_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leaflist_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_list_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_anydata_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_choice_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_uses_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_extension_sub_yin, setup_ctx_yin, teardown_ctx),

//        cmocka_unit_test_setup_teardown(test_fullset_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_module_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_container_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_leaf_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_leaflist_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_list_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_anydata_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_choice_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_uses_sub_yang, setup_ctx_yang, teardown_ctx),
//        cmocka_unit_test_setup_teardown(test_extension_sub_yang, setup_ctx_yang, teardown_ctx),
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
