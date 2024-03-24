var srs =
[
    [ "Purpose and Scope", "srs.html#srs_scope", [
      [ "Revision History", "srs.html#srs_history", null ],
      [ "About QP Framework", "srs.html#srs_about", null ],
      [ "QP Framework and Functional Safety", "srs.html#srs_fs", null ],
      [ "Audience", "srs.html#srs_audience", null ],
      [ "Document Conventions", "srs.html#srs_conv", null ],
      [ "General Requirement UIDs", "srs.html#srs_uid", null ],
      [ "Use of Shall/Should/etc.", "srs.html#srs_lang", [
        [ "\"shall\"", "srs.html#srs_lang_shall", null ],
        [ "\"should\"", "srs.html#srs_lang-should", null ],
        [ "\"may\"", "srs.html#srs_lang-may", null ],
        [ "\"must not\"", "srs.html#srs_lang-must-not", null ]
      ] ],
      [ "Document Organization", "srs.html#srs_org", null ],
      [ "References", "srs.html#srs_ref", null ]
    ] ],
    [ "Overview", "srs_over.html", [
      [ "Goals", "srs_over.html#srs_over-goals", null ],
      [ "Object-Orientation", "srs_over.html#srs_over_oo", null ],
      [ "Context and Functional Decomposition", "srs_over.html#srs_over-ctxt", [
        [ "Active Objects", "srs_over.html#srs_intro_ao", null ],
        [ "State Machines", "srs_over.html#srs_over_sm", null ],
        [ "Event-Driven Paradigm", "srs_over.html#srs_over_ed", null ],
        [ "Inversion of Control", "srs_over.html#srs_over_inv", null ],
        [ "Framework, NOT a Library", "srs_over.html#srs_over_frame", null ]
      ] ],
      [ "Portability & Configurability", "srs_over.html#srs_over-conf", [
        [ "Compile-Time Configurability", "srs_over.html#srs_conf_compile", null ],
        [ "Run-Time Configurability", "srs_over.html#srs_conf_run", null ]
      ] ]
    ] ],
    [ "Events", "srs_evt.html", [
      [ "Concepts & Definitions", "srs_evt.html#srs_evt-def", [
        [ "Event Signal", "srs_evt.html#srs_evt-sig", null ],
        [ "Event Parameters", "srs_evt.html#srs_evt-par", null ]
      ] ],
      [ "Requirements", "srs_evt.html#srs_evt-req", [
        [ "REQ-QP-01_00", "srs_evt.html#REQ-QP-01_00", null ],
        [ "REQ-QP-01_20", "srs_evt.html#REQ-QP-01_20", null ],
        [ "REQ-QP-01_21", "srs_evt.html#REQ-QP-01_21", null ],
        [ "REQ-QP-01_22", "srs_evt.html#REQ-QP-01_22", null ],
        [ "REQ-QP-01_23", "srs_evt.html#REQ-QP-01_23", null ],
        [ "REQ-QP-01_30", "srs_evt.html#REQ-QP-01_30", null ],
        [ "REQ-QP-01_31", "srs_evt.html#REQ-QP-01_31", null ],
        [ "REQ-QP-01_40", "srs_evt.html#REQ-QP-01_40", null ],
        [ "REQ-QP-01_41", "srs_evt.html#REQ-QP-01_41", null ]
      ] ]
    ] ],
    [ "State Machines", "srs_sm.html", [
      [ "Concepts & Definitions", "srs_sm.html#srs_sm-intro", [
        [ "State", "srs_sm.html#srs_sm-state", null ],
        [ "Transition", "srs_sm.html#srs_sm-tran", null ],
        [ "State Machine", "srs_sm.html#srs_sm-def", null ],
        [ "Hierarchical State Machine", "srs_sm.html#srs_sm-hsm", null ],
        [ "State Machine Implementation Strategy", "srs_sm.html#srs_sm-impl", null ],
        [ "Dispatching Events to a State Machine in QP Framework", "srs_sm.html#srs_sm-dispatch", [
          [ "State Machine Specification", "srs_sm.html#srs_sm-spec", null ],
          [ "State Machine Processor", "srs_sm.html#srs_sm-proc", null ],
          [ "Run To Completion (RTC) Processing", "srs_sm.html#srs_sm-rtc", null ],
          [ "Current Event", "srs_sm.html#srs_sm-curr", null ]
        ] ]
      ] ],
      [ "Requirements", "srs_sm.html#srs_sm-req", [
        [ "REQ-QP-02_00", "srs_sm.html#REQ-QP-02_00", null ],
        [ "REQ-QP-02_10", "srs_sm.html#REQ-QP-02_10", null ],
        [ "REQ-QP-02_20", "srs_sm.html#REQ-QP-02_20", null ],
        [ "REQ-QP-02_21", "srs_sm.html#REQ-QP-02_21", null ],
        [ "REQ-QP-02_22", "srs_sm.html#REQ-QP-02_22", null ],
        [ "REQ-QP-02_23", "srs_sm.html#REQ-QP-02_23", null ],
        [ "REQ-QP-02_24", "srs_sm.html#REQ-QP-02_24", null ],
        [ "REQ-QP-02_25", "srs_sm.html#REQ-QP-02_25", null ],
        [ "REQ-QP-02_30", "srs_sm.html#REQ-QP-02_30", null ],
        [ "REQ-QP-02_31", "srs_sm.html#REQ-QP-02_31", null ],
        [ "REQ-QP-02_32", "srs_sm.html#REQ-QP-02_32", null ],
        [ "REQ-QP-02_33", "srs_sm.html#REQ-QP-02_33", null ],
        [ "REQ-QP-02_34", "srs_sm.html#REQ-QP-02_34", null ],
        [ "REQ-QP-02_35", "srs_sm.html#REQ-QP-02_35", null ],
        [ "REQ-QP-02_36", "srs_sm.html#REQ-QP-02_36", null ],
        [ "REQ-QP-02_37", "srs_sm.html#REQ-QP-02_37", null ],
        [ "REQ-QP-02_38", "srs_sm.html#REQ-QP-02_38", null ],
        [ "REQ-QP-02_39", "srs_sm.html#REQ-QP-02_39", null ],
        [ "REQ-QP-02_40", "srs_sm.html#REQ-QP-02_40", null ],
        [ "REQ-QP-02_50", "srs_sm.html#REQ-QP-02_50", null ],
        [ "REQ-QP-02_51", "srs_sm.html#REQ-QP-02_51", null ],
        [ "REQ-QP-02_52", "srs_sm.html#REQ-QP-02_52", null ],
        [ "REQ-QP-02_53", "srs_sm.html#REQ-QP-02_53", null ],
        [ "REQ-QP-02_54", "srs_sm.html#REQ-QP-02_54", null ],
        [ "REQ-QP-02_55", "srs_sm.html#REQ-QP-02_55", null ],
        [ "REQ-QP-02_56", "srs_sm.html#REQ-QP-02_56", null ]
      ] ]
    ] ],
    [ "Active Objects", "srs_ao.html", [
      [ "Concepts & Definitions", "srs_ao.html#srs_ao-intro", [
        [ "Active Object Model of Computation", "srs_ao.html#srs_ao-model", null ],
        [ "Active Objects", "srs_ao.html#srs_ao-def", null ],
        [ "Encapsulation for Concurrency", "srs_ao.html#srs_ao-enc", null ],
        [ "Execution Context", "srs_ao.html#srs_ao-exec", null ],
        [ "Priority", "srs_ao.html#srs_ao-prio", null ],
        [ "Event Queue", "srs_ao.html#srs_ao-queue", null ],
        [ "Asynchronous Communication", "srs_ao.html#srs_ao-asynch", null ],
        [ "Run-to-Completion (RTC)", "srs_ao.html#srs_ao-rtc", null ],
        [ "Current Event", "srs_ao.html#srs_ao-curr", null ],
        [ "No Blocking", "srs_ao.html#srs_ao-block", null ],
        [ "Support For State Machines", "srs_ao.html#srs_ao-sm", null ],
        [ "Inversion of Control", "srs_ao.html#srs_ao-inv", null ],
        [ "Framework vs. Toolkit", "srs_ao.html#srs_ao-frame", null ],
        [ "Low Power Architecture", "srs_ao.html#srs_ao-power", null ]
      ] ],
      [ "Requirements", "srs_ao.html#srs_ao-req", [
        [ "REQ-QP-03_00", "srs_ao.html#REQ-QP-03_00", null ],
        [ "REQ-QP-03_01", "srs_ao.html#REQ-QP-03_01", null ],
        [ "REQ-QP-03_10", "srs_ao.html#REQ-QP-03_10", null ],
        [ "REQ-QP-03_11", "srs_ao.html#REQ-QP-03_11", null ],
        [ "REQ-QP-03_20", "srs_ao.html#REQ-QP-03_20", null ],
        [ "REQ-QP-03_21", "srs_ao.html#REQ-QP-03_21", null ],
        [ "REQ-QP-03_22", "srs_ao.html#REQ-QP-03_22", null ],
        [ "REQ-QP-03_23", "srs_ao.html#REQ-QP-03_23", null ],
        [ "REQ-QP-03_30", "srs_ao.html#REQ-QP-03_30", null ],
        [ "REQ-QP-03_31", "srs_ao.html#REQ-QP-03_31", null ],
        [ "REQ-QP-03_32", "srs_ao.html#REQ-QP-03_32", null ],
        [ "REQ-QP-03_40", "srs_ao.html#REQ-QP-03_40", null ],
        [ "REQ-QP-03_50", "srs_ao.html#REQ-QP-03_50", null ],
        [ "REQ-QP-03_51", "srs_ao.html#REQ-QP-03_51", null ],
        [ "REQ-QP-03_60", "srs_ao.html#REQ-QP-03_60", null ],
        [ "REQ-QP-03_70", "srs_ao.html#REQ-QP-03_70", null ],
        [ "REQ-QP-03_80", "srs_ao.html#REQ-QP-03_80", null ]
      ] ]
    ] ],
    [ "Event Delivery Mechanisms", "srs_edm.html", [
      [ "Concepts & Definitions", "srs_edm.html#srs_edm-intro", [
        [ "Direct Event Posting", "srs_edm.html#src_edm-post", null ],
        [ "Publish-Subscribe", "srs_edm.html#src_edm-ps", null ],
        [ "Event Delivery Guarantee", "srs_edm.html#src_edm-guarantee", null ]
      ] ],
      [ "Requirements", "srs_edm.html#srs_edm-req", [
        [ "REQ-QP-04_00", "srs_edm.html#REQ-QP-04_00", null ],
        [ "REQ-QP-04_01", "srs_edm.html#REQ-QP-04_01", null ],
        [ "REQ-QP-04_10", "srs_edm.html#REQ-QP-04_10", null ],
        [ "REQ-QP-04_20", "srs_edm.html#REQ-QP-04_20", null ],
        [ "REQ-QP-04_21", "srs_edm.html#REQ-QP-04_21", null ],
        [ "REQ-QP-04_50", "srs_edm.html#REQ-QP-04_50", null ],
        [ "REQ-QP-04_51", "srs_edm.html#REQ-QP-04_51", null ],
        [ "REQ-QP-04_52", "srs_edm.html#REQ-QP-04_52", null ],
        [ "REQ-QP-04_53", "srs_edm.html#REQ-QP-04_53", null ],
        [ "REQ-QP-04_54", "srs_edm.html#REQ-QP-04_54", null ],
        [ "REQ-QP-04_55", "srs_edm.html#REQ-QP-04_55", null ],
        [ "REQ-QP-04_80", "srs_edm.html#REQ-QP-04_80", null ],
        [ "REQ-QP-04_81", "srs_edm.html#REQ-QP-04_81", null ]
      ] ]
    ] ],
    [ "Event Memory Management", "srs_emm.html", [
      [ "Concepts & Definitions", "srs_emm.html#srs_emm-def", [
        [ "Immutable Events", "srs_emm.html#src_emm-imm", null ],
        [ "Mutable Events", "srs_emm.html#srs_emm-mut", null ],
        [ "Zero-Copy Event Management", "srs_emm.html#src_emm-zero", null ],
        [ "Mutable Event Ownership Rules", "srs_emm.html#src_emm-ownership", null ],
        [ "Event Pools", "srs_emm.html#src_emm-pools", null ]
      ] ],
      [ "Requirements", "srs_emm.html#src_emm-req", [
        [ "REQ-QP-05_00", "srs_emm.html#REQ-QP-05_00", null ],
        [ "REQ-QP-05_10", "srs_emm.html#REQ-QP-05_10", null ],
        [ "REQ-QP-05_20", "srs_emm.html#REQ-QP-05_20", null ],
        [ "REQ-QP-05_21", "srs_emm.html#REQ-QP-05_21", null ],
        [ "REQ-QP-05_22", "srs_emm.html#REQ-QP-05_22", null ],
        [ "REQ-QP-05_30", "srs_emm.html#REQ-QP-05_30", null ],
        [ "REQ-QP-05_40", "srs_emm.html#REQ-QP-05_40", null ]
      ] ]
    ] ],
    [ "Time Management", "srs_tm.html", [
      [ "Concepts & Definitions", "srs_tm.html#srs_tm-intro", [
        [ "Time Events", "srs_tm.html#srs_tm-te", null ],
        [ "System Clock Tick", "srs_tm.html#srs_tm-tick", null ],
        [ "\"Tickless Mode\"", "srs_tm.html#srs_tm-tickless", null ],
        [ "Multiple Tick Rates", "srs_tm.html#srs_tm-rates", null ]
      ] ],
      [ "Requirements", "srs_tm.html#srs_tm-req", [
        [ "REQ-QP-06_00", "srs_tm.html#REQ-QP-06_00", null ]
      ] ]
    ] ],
    [ "Software Tracing", "srs_qs.html", [
      [ "Concepts & Definitions", "srs_qs.html#srs_qs-def", null ],
      [ "Requirements", "srs_qs.html#srs_qs-req", null ]
    ] ],
    [ "Cooperative Run-to-Completion Kernel", "srs_qv.html", [
      [ "Theory of Operation", "srs_qv.html#srs_qv-theory", null ],
      [ "Requirements", "srs_qv.html#srs_qv-req", [
        [ "REQ-QP-07_00", "srs_qv.html#REQ-QP-07_00", null ]
      ] ]
    ] ],
    [ "Preemptive Run-to-Completion Kernel", "srs_qk.html", [
      [ "Theory of Operation", "srs_qk.html#srs_qk-theory", null ],
      [ "QK Features", "srs_qk.html#qsrs_qk-feat", [
        [ "Scheduler Locking", "srs_qk.html#qsrs_qk-lock", null ]
      ] ],
      [ "Requirements", "srs_qk.html#srs_qk-req", [
        [ "REQ-QP-08_00", "srs_qk.html#REQ-QP-08_00", null ],
        [ "REQ-QP-08_10", "srs_qk.html#REQ-QP-08_10", null ]
      ] ]
    ] ],
    [ "Preemptive Dual-Mode Kernel", "srs_qxk.html", [
      [ "Theory of Operation", "srs_qxk.html#srs_qxk-theory", null ],
      [ "Basic Threads", "srs_qxk.html#srs_qxk-basic", null ],
      [ "Extended Threads", "srs_qxk.html#srs_qxk-ext", null ],
      [ "QXK Feature Summary", "srs_qxk.html#srs_qxk-feat", [
        [ "Scheduler Locking", "srs_qxk.html#srs_qxk-lock", null ],
        [ "Thread Local Storage", "srs_qxk.html#srs_qxk-tls", null ]
      ] ],
      [ "Requirements", "srs_qxk.html#srs_qxk-req", null ]
    ] ],
    [ "Quality Attributes", "srs_qa.html", [
      [ "Compliance with Standards", "srs_qa.html#srs_qa-std", null ],
      [ "Software Quality Attributes", "srs_qa.html#srs_qa-quality", null ],
      [ "Performance Requirements", "srs_qa.html#srs_qa-perform", null ],
      [ "Portability Requirements", "srs_qa.html#srs_qa-port", null ],
      [ "Ease of Development Requirements", "srs_qa.html#srs_qa-develop", [
        [ "REQ-QP-09_01", "srs_qa.html#REQ-QP-09_01", null ],
        [ "REQ-QP-09_02", "srs_qa.html#REQ-QP-09_02", null ],
        [ "REQ-QP-09_03", "srs_qa.html#REQ-QP-09_03", null ]
      ] ]
    ] ]
];