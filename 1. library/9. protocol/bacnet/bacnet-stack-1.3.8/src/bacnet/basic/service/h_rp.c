/**************************************************************************
 *
 * Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
 *
 * SPDX-License-Identifier: MIT
 *
 *********************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
/* BACnet Stack defines - first */
#include "bacnet/bacdef.h"
/* BACnet Stack API */
#include "bacnet/bacdcode.h"
#include "bacnet/bacerror.h"
#include "bacnet/bacdevobjpropref.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "bacnet/abort.h"
#include "bacnet/reject.h"
#include "bacnet/rp.h"
/* basic objects, services, TSM, and datalink */
#include "bacnet/basic/object/device.h"
#if (BACNET_PROTOCOL_REVISION >= 17)
#include "bacnet/basic/object/netport.h"
#endif
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/basic/services.h"
#include "bacnet/datalink/datalink.h"

/** @file h_rp.c  Handles Read Property requests. */

/** Handler for a ReadProperty Service request.
 * @ingroup DSRP
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 *   - if the response would be too large
 * - the result from Device_Read_Property(), if it succeeds
 * - an Error if Device_Read_Property() fails
 *   or there isn't enough room in the APDU to fit the data.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_read_property(uint8_t *service_request,
    uint16_t service_len,
    BACNET_ADDRESS *src,
    BACNET_CONFIRMED_SERVICE_DATA *service_data)
{
    BACNET_READ_PROPERTY_DATA rpdata;
    int len = 0;
    int pdu_len = 0;
    int apdu_len = -1;
    int npdu_len = -1;
    BACNET_NPDU_DATA npdu_data;
    bool error = true; /* assume that there is an error */
    int bytes_sent = 0;
    BACNET_ADDRESS my_address;

    /* configure default error code as an abort since it is common */
    rpdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
    /* encode the NPDU portion of the packet */
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, service_data->priority);
    npdu_len = npdu_encode_pdu(
        &Handler_Transmit_Buffer[0], src, &my_address, &npdu_data);
    if (npdu_len <= 0) {
        /* If 0 or negative, there were problems with the data or encoding. */
        len = BACNET_STATUS_ABORT;
#if PRINT_ENABLED
        fprintf(stderr, "RP: npdu_encode_pdu error.  Sending Abort!\n");
#endif
    } else if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len = BACNET_STATUS_ABORT;
#if PRINT_ENABLED
        fprintf(stderr, "RP: Segmented message.  Sending Abort!\n");
#endif
    } else {
        len = rp_decode_service_request(service_request, service_len, &rpdata);
#if PRINT_ENABLED
        if (len <= 0) {
            fprintf(stderr, "RP: Unable to decode Request!\n");
        }
#endif
        if (len < 0) {
            /* bad decoding - skip to error/reject/abort handling */
            error = true;
#if PRINT_ENABLED
            fprintf(stderr, "RP: Bad Encoding.\n");
#endif
        } else {
            /* When the object-type in the Object Identifier parameter
               contains the value DEVICE and the instance in the 'Object
               Identifier' parameter contains the value 4194303, the responding
               BACnet-user shall treat the Object Identifier as if it correctly
               matched the local Device object. This allows the device instance
               of a device that does not generate I-Am messages to be
               determined. */
            if ((rpdata.object_type == OBJECT_DEVICE) &&
                (rpdata.object_instance == BACNET_MAX_INSTANCE)) {
                rpdata.object_instance = Device_Object_Instance_Number();
            }
#if (BACNET_PROTOCOL_REVISION >= 17)
            /* When the object-type in the Object Identifier parameter
               contains the value NETWORK_PORT and the instance in the 'Object
               Identifier' parameter contains the value 4194303, the responding
               BACnet-user shall treat the Object Identifier as if it correctly
               matched the local Network Port object representing the network
               port through which the request was received. This allows the
               network port instance of the network port that was used to
               receive the request to be determined. */
            if ((rpdata.object_type == OBJECT_NETWORK_PORT) &&
                (rpdata.object_instance == BACNET_MAX_INSTANCE)) {
                rpdata.object_instance = Network_Port_Index_To_Instance(0);
            }
#endif
            apdu_len =
                rp_ack_encode_apdu_init(&Handler_Transmit_Buffer[npdu_len],
                    service_data->invoke_id, &rpdata);
            /* configure our storage */
            rpdata.application_data =
                &Handler_Transmit_Buffer[npdu_len + apdu_len];
            rpdata.application_data_len =
                sizeof(Handler_Transmit_Buffer) - (npdu_len + apdu_len);
            len = Device_Read_Property(&rpdata);
            if (len >= 0) {
                apdu_len += len;
                len = rp_ack_encode_apdu_object_property_end(
                    &Handler_Transmit_Buffer[npdu_len + apdu_len]);
                apdu_len += len;
                if (apdu_len > service_data->max_resp) {
                    /* too big for the sender - send an abort!
                       Setting of error code needed here as read property
                       processing may have overridden the default set at start
                     */
                    rpdata.error_code =
                        ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                    len = BACNET_STATUS_ABORT;
#if PRINT_ENABLED
                    fprintf(stderr, "RP: Message too large.\n");
#endif
                } else {
#if PRINT_ENABLED
                    fprintf(stderr, "RP: Sending Ack!\n");
#endif
                    error = false;
                }
            } else {
#if PRINT_ENABLED
                fprintf(stderr, "RP: Device_Read_Property: ");
                if (len == BACNET_STATUS_ABORT) {
                    fprintf(stderr, "Abort!\n");
                } else if (len == BACNET_STATUS_ERROR) {
                    fprintf(stderr, "Error!\n");
                } else if (len == BACNET_STATUS_REJECT) {
                    fprintf(stderr, "Reject!\n");
                } else {
                    fprintf(stderr, "Unknown Len=%d\n", len);
                }
#endif
            }
        }
    }

    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            apdu_len = abort_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                service_data->invoke_id,
                abort_convert_error_code(rpdata.error_code), true);
#if PRINT_ENABLED
            fprintf(stderr, "RP: Sending Abort!\n");
#endif
        } else if (len == BACNET_STATUS_ERROR) {
            apdu_len = bacerror_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                service_data->invoke_id, SERVICE_CONFIRMED_READ_PROPERTY,
                rpdata.error_class, rpdata.error_code);
#if PRINT_ENABLED
            fprintf(stderr, "RP: Sending Error!\n");
#endif
        } else if (len == BACNET_STATUS_REJECT) {
            apdu_len = reject_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                service_data->invoke_id,
                reject_convert_error_code(rpdata.error_code));
#if PRINT_ENABLED
            fprintf(stderr, "RP: Sending Reject!\n");
#endif
        }
    }

    pdu_len = npdu_len + apdu_len;
    bytes_sent = datalink_send_pdu(
        src, &npdu_data, &Handler_Transmit_Buffer[0], pdu_len);
    if (bytes_sent <= 0) {
#if PRINT_ENABLED
        fprintf(stderr, "Failed to send PDU (%s)!\n", strerror(errno));
#endif
    }

    return;
}