//$file${src::qf::qf_qeq.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpc.qm
// File:  ${src::qf::qf_qeq.c}
//
// This code has been generated by QM 6.2.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C real-time embedded framework
// Framework(s) : qpc
// Support ends : 2025-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${src::qf::qf_qeq.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#define QP_IMPL           // this is QP implementation
#include "qp_port.h"      // QP port
#include "qp_pkg.h"       // QP package-scope interface
#include "qsafe.h"        // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY              // QS software tracing enabled?
    #include "qs_port.h"  // QS port
    #include "qs_pkg.h"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.h" // disable the QS software tracing
#endif // Q_SPY

Q_DEFINE_THIS_MODULE("qf_qeq")

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U)%0x2710U))
#error qpc version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QEQueue} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${QF::QEQueue} .............................................................
QEQueue QEQueue_dummy;

//${QF::QEQueue::init} .......................................................
//! @public @memberof QEQueue
void QEQueue_init(QEQueue * const me,
    struct QEvt const ** const qSto,
    uint_fast16_t const qLen)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    #if (QF_EQUEUE_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(100, qLen < 0xFFU);
    #endif

    me->frontEvt = (QEvt *)0; // no events in the queue
    me->ring     = qSto;      // the beginning of the ring buffer
    me->end      = (QEQueueCtr)qLen;
    if (qLen != 0U) {
        me->head = 0U;
        me->tail = 0U;
    }
    me->nFree    = (QEQueueCtr)(qLen + 1U); // +1 for frontEvt
    me->nMin     = me->nFree;

    QF_MEM_APP();
    QF_CRIT_EXIT();
}

//${QF::QEQueue::post} .......................................................
//! @public @memberof QEQueue
bool QEQueue_post(QEQueue * const me,
    struct QEvt const * const e,
    uint_fast16_t const margin,
    uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    Q_REQUIRE_INCRIT(200, QEvt_verify_(e));

    QEQueueCtr nFree = me->nFree; // get volatile into temporary

    // required margin available?
    bool status;
    if (((margin == QF_NO_MARGIN) && (nFree > 0U))
        || (nFree > (QEQueueCtr)margin))
    {
        // is it a mutable event?
        if (QEvt_getPoolNum_(e) != 0U) {
            QEvt_refCtr_inc_(e); // increment the reference counter
        }

        --nFree; // one free entry just used up
        me->nFree = nFree; // update the original
        if (me->nMin > nFree) {
            me->nMin = nFree; // update minimum so far
        }

        QS_BEGIN_PRE_(QS_QF_EQUEUE_POST, qsId)
            QS_TIME_PRE_();        // timestamp
            QS_SIG_PRE_(e->sig);   // the signal of this event
            QS_OBJ_PRE_(me);       // this queue object
            QS_2U8_PRE_(QEvt_getPoolNum_(e), e->refCtr_); // poolNum & refCtr
            QS_EQC_PRE_(nFree);    // # free entries
            QS_EQC_PRE_(me->nMin); // min # free entries
        QS_END_PRE_()

        if (me->frontEvt == (QEvt *)0) { // was the queue empty?
            me->frontEvt = e; // deliver event directly
        }
        else { // queue was not empty, insert event into the ring-buffer
            // insert event into the ring buffer (FIFO)...
            me->ring[me->head] = e; // insert e into buffer
            // need to wrap the head?
            if (me->head == 0U) {
                me->head = me->end; // wrap around
            }
            --me->head;
        }
        status = true; // event posted successfully
    }
    else { // event cannot be posted
        // dropping events must be acceptable
        Q_ASSERT_INCRIT(210, margin != QF_NO_MARGIN);

        QS_BEGIN_PRE_(QS_QF_EQUEUE_POST_ATTEMPT, qsId)
            QS_TIME_PRE_();        // timestamp
            QS_SIG_PRE_(e->sig);   // the signal of this event
            QS_OBJ_PRE_(me);       // this queue object
            QS_2U8_PRE_(QEvt_getPoolNum_(e), e->refCtr_); // poolNum & refCtr
            QS_EQC_PRE_(nFree);    // # free entries
            QS_EQC_PRE_(margin);   // margin requested
        QS_END_PRE_()

        status = false; // event not posted
    }

    QF_MEM_APP();
    QF_CRIT_EXIT();

    return status;
}

//${QF::QEQueue::postLIFO} ...................................................
//! @public @memberof QEQueue
void QEQueue_postLIFO(QEQueue * const me,
    struct QEvt const * const e,
    uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    Q_REQUIRE_INCRIT(300, QEvt_verify_(e));

    QEQueueCtr nFree = me->nFree; // get volatile into temporary
    Q_REQUIRE_INCRIT(301, nFree != 0U);

    if (QEvt_getPoolNum_(e) != 0U) { // is it a mutable event?
        QEvt_refCtr_inc_(e); // increment the reference counter
    }

    --nFree; // one free entry just used up
    me->nFree = nFree; // update the original
    if (me->nMin > nFree) {
        me->nMin = nFree; // update minimum so far
    }

    QS_BEGIN_PRE_(QS_QF_EQUEUE_POST_LIFO, qsId)
        QS_TIME_PRE_();         // timestamp
        QS_SIG_PRE_(e->sig);    // the signal of this event
        QS_OBJ_PRE_(me);        // this queue object
        QS_2U8_PRE_(QEvt_getPoolNum_(e), e->refCtr_); // poolNum & refCtr
        QS_EQC_PRE_(nFree);     // # free entries
        QS_EQC_PRE_(me->nMin);  // min # free entries
    QS_END_PRE_()

    QEvt const * const frontEvt = me->frontEvt; // read into temporary
    me->frontEvt = e; // deliver event directly to the front of the queue

    if (frontEvt != (QEvt *)0) { // was the queue not empty?
        ++me->tail;
        if (me->tail == me->end) { // need to wrap the tail?
            me->tail = 0U; // wrap around
        }
        me->ring[me->tail] = frontEvt; // save old front evt
    }

    QF_MEM_APP();
    QF_CRIT_EXIT();
}

//${QF::QEQueue::get} ........................................................
//! @public @memberof QEQueue
struct QEvt const * QEQueue_get(QEQueue * const me,
    uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    QEvt const * const e = me->frontEvt; // always remove evt from the front

    if (e != (QEvt *)0) { // was the queue not empty?
        Q_INVARIANT_INCRIT(412, QEvt_verify_(e));

        // use a temporary variable to increment me->nFree
        QEQueueCtr const nFree = me->nFree + 1U;
        me->nFree = nFree; // update the # free

        // any events in the ring buffer?
        if (nFree <= me->end) {
            me->frontEvt = me->ring[me->tail]; // get from tail
            if (me->tail == 0U) { // need to wrap the tail?
                me->tail = me->end; // wrap around
            }
            --me->tail;

            QS_BEGIN_PRE_(QS_QF_EQUEUE_GET, qsId)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(me);     // this queue object
                QS_2U8_PRE_(QEvt_getPoolNum_(e), e->refCtr_); // poolNum & refCtr
                QS_EQC_PRE_(nFree);  // # free entries
            QS_END_PRE_()
        }
        else {
            me->frontEvt = (QEvt *)0; // queue becomes empty

            // all entries in the queue must be free (+1 for fronEvt)
            Q_INVARIANT_INCRIT(420, nFree == (me->end + 1U));

            QS_BEGIN_PRE_(QS_QF_EQUEUE_GET_LAST, qsId)
                QS_TIME_PRE_();      // timestamp
                QS_SIG_PRE_(e->sig); // the signal of this event
                QS_OBJ_PRE_(me);     // this queue object
                QS_2U8_PRE_(QEvt_getPoolNum_(e), e->refCtr_); // poolNum & refCtr
            QS_END_PRE_()
        }
    }

    QF_MEM_APP();
    QF_CRIT_EXIT();

    return e;
}
//$enddef${QF::QEQueue} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^