//    _   _             _    _  _____ ____
//   | | (_)           | |  | |/ ____|  _ \   _     _
//   | |_ _ _ __  _   _| |  | | (___ | |_) |_| |_ _| |_
//   | __| | '_ \| | | | |  | |\___ \|  _ < _   _|_   _|
//   | |_| | | | | |_| | |__| |____) | |_) | |_|   |_|
//    \__|_|_| |_|\__, |\____/|_____/|____/
//                __/ |
//               |___/
//
// This file is part of tinyUSB++, C++ based and easy to
// use library for USB host/device functionality.
// (c) 2024 A. Terstegge  (Andreas.Terstegge@gmail.com)
//
#include "usb_msc_bot_device.h"
#include "usb_structs.h"
#include "usb_log.h"

#include <cassert>
#include <machine/endian.h>

using namespace USB;
using enum USB::bInterfaceClass_t;
using enum USB::bInterfaceSubClass_t;
using enum USB::bInterfaceProtocol_t;
using enum USB::ep_attributes_t;
using enum USB::direction_t;
using enum SCSI::peripheral_device_type_t;
using enum SCSI::peripheral_qualifier_type_t;

usb_msc_bot_device::usb_msc_bot_device(
        usb_device_controller & controller,
        usb_configuration     & configuration)
:  _configuration(configuration), _max_lun(0), _state(state_t::RECEIVE_CBW), _buffer_out_len(0)
{
    // USB interface descriptor config
    //////////////////////////////////
    _interface.set_bInterfaceClass   (IF_CLASS_MSC);
    _interface.set_bInterfaceSubClass(IF_SUBCLASS_SCSI_TRANSPARENT);
    _interface.set_bInterfaceProtocol(IF_PROTOCOL_MSC_BOT);

    // USB endpoints
    ////////////////
    _ep_in  = controller.create_endpoint(_interface, DIR_IN,  TRANS_BULK);
    _ep_out = controller.create_endpoint(_interface, DIR_OUT, TRANS_BULK);

    // Prepare new request to receive data
    //////////////////////////////////////
    _ep_out->start_transfer(_buffer_out, TUPP_MSC_BLOCK_SIZE);

    // Endpoint handler
    ///////////////////
    _ep_out->data_handler = [&](uint8_t *, uint16_t len) {
        if (_buffer_out_len) {
            TUPP_LOG(LOG_WARNING, "Unconsumed data!");
        }
        // New data has arrived from the host! Stop incoming data ...
        _ep_out->send_NAK(true);
        // ... and set length to signal new data
        _buffer_out_len = len;
        // Finally trigger a new reception
        _ep_out->start_transfer(_buffer_out, TUPP_MSC_BLOCK_SIZE);
    };

    // Handler for CDC ACM specific requests
    ////////////////////////////////////////
    _interface.setup_handler = [&](USB::setup_packet_t *pkt) {
        switch(pkt->bRequest) {
            case bRequest_t::REQ_MSC_BOT_RESET: {
                TUPP_LOG(LOG_INFO, "REQ_MSC_BOT_RESET");
                assert(pkt->wValue  == 0);
                assert(pkt->wLength == 0);
                break;
            }
            case bRequest_t::REQ_MSC_GET_MAX_LUN: {
                TUPP_LOG(LOG_INFO, "REQ_MSC_GET_MAX_LUN");
                assert(pkt->wValue  == 0);
                assert(pkt->wLength == 1);
                controller._ep0_in->send_stall(true);
//                controller._ep0_in->start_transfer(&_max_lun, 1);
                break;
            }
            default: {
                TUPP_LOG(LOG_ERROR, "Unsupported MSC command 0x%x", pkt->bRequest);
            }
        }
    };

    // Initialize SCSI inquiry response
    ///////////////////////////////////
    _inquiry_response.peripheral_device    = SBC_4_DIRECT_ACCESS;
    _inquiry_response.peripheral_qualifier = DEVICE_CONNECTED_TO_LUN;
    _inquiry_response.removable_media      = 1;
    _inquiry_response.version              = SCSI::version_t::NO_STANDARD;
    _inquiry_response.response_data_format = 2;
//    _inquiry_response.hierarchical_support = 0;
//    _inquiry_response.normal_aca_support   = 0;
    _inquiry_response.additional_length    = sizeof(SCSI::inquiry_response_t) - 5;
//    _inquiry_response.protect              = 0;
//    _inquiry_response.third_party_copy     = 0;
//    _inquiry_response.access_controls_coordinator = 0;
//    _inquiry_response.scc_support          = 0;
//    _inquiry_response.multi_port           = 0;
//    _inquiry_response.enclosure_services   = 0;
//    _inquiry_response.command_queuing      = 0;
}

// This method implements a simple state machine
// according to the MSC BOT specification
void usb_msc_bot_device::handle_request() {
    switch(_state) {
        case state_t::RECEIVE_CBW: {
            // Did we receive some data?
            if (_buffer_out_len) {
                TUPP_LOG(LOG_DEBUG, "STATE: RECEIVE_CBW");
                // Create pointer to CBW
                MSC::cbw_t *cbw = (MSC::cbw_t *) _buffer_out;

                // Prepare the command status wrapper
                _csw.dCSWSignature   = MSC::csw_signature;
                _csw.dCSWTag         = cbw->dCBWTag;
                _csw.dCSWDataResidue = 0;
                _csw.bCSWStatus      = MSC::csw_status_t::CMD_PASSED;

                // Check size of data
                if ((_buffer_out_len    != sizeof(MSC::cbw_t)) ||
                    (cbw->dCBWSignature != MSC::cbw_signature)) {
                    // Prepare the command status wrapper
                    _csw.dCSWSignature   = MSC::csw_signature;
                    _csw.dCSWTag         = cbw->dCBWTag;
                    _csw.dCSWDataResidue = 0;
                    _csw.bCSWStatus      = MSC::csw_status_t::PHASE_ERROR;

                    _ep_out->send_NAK(false);
                    // Signal that data has been processed
                    _buffer_out_len = 0;
                    // Continue with sending the CSW
                    _state = state_t::SEND_CSW;
                    return;
                }

                // Continue with sending the CSW
                _state = state_t::SEND_CSW;

                process_scsi_command();

                // Signal that data has been processed
                _buffer_out_len = 0;
                _ep_out->send_NAK(false);
            }
            break;
        }
        case state_t::DATA_READ: {
            if (_ep_in->is_active()) {
                break;
            }
            TUPP_LOG(LOG_DEBUG, "STATE: DATA_READ");
            // Read data from device
            read_handler(_buffer_in, _block_addr++);
            _ep_in->start_transfer(_buffer_in, TUPP_MSC_BLOCK_SIZE);
            _blocks_transferred++;
            if (_blocks_transferred == _blocks_to_transfer) {
                _state = state_t::SEND_CSW;
            }
            break;
        }
        case state_t::DATA_WRITE: {
            if (_buffer_out_len == 0) {
                break;
            }
            TUPP_LOG(LOG_DEBUG, "STATE: DATA_WRITE");
            assert(_buffer_out_len == TUPP_MSC_BLOCK_SIZE);
            // Write data to device
            write_handler(_buffer_out, _block_addr++);
            _blocks_transferred++;
            // Mark as consumed
            _buffer_out_len = 0;
            if (_blocks_transferred == _blocks_to_transfer) {
                _state = state_t::SEND_CSW;
            }
            _buffer_out_len = 0;
            _ep_out->send_NAK(false);
            break;
        }
        case state_t::SEND_CSW: {
            TUPP_LOG(LOG_DEBUG, "STATE: SEND_CSW");
            activity_handler(false, false);
            if (_ep_in->is_active()) {
                break;
            }
            _ep_in->start_transfer((uint8_t *)&_csw, sizeof(_csw));
            _state = state_t::RECEIVE_CBW;
            break;
        }
    }
}

void usb_msc_bot_device::process_scsi_command() {
    TUPP_LOG(LOG_DEBUG, "process_scsi_command()");
    MSC::cbw_t       * cbw = (MSC::cbw_t *) _buffer_out;
    SCSI::scsi_cmd_t * cmd = (SCSI::scsi_cmd_t *)&cbw->CBWCB;

    switch(*cmd) {
        case SCSI::scsi_cmd_t::TEST_UNIT_READY: {
            TUPP_LOG(LOG_INFO, "SCSI: TEST_UNIT_READY");
            assert(cbw->bCBWCBLength == sizeof(SCSI::test_unit_ready_t));
            assert(cbw->dCBWDataTransferLength == 0);
            break;
        }
        case SCSI::scsi_cmd_t::INQUIRY: {
            TUPP_LOG(LOG_INFO, "SCSI: INQUIRY");
//            assert(cbw->bCBWCBLength == sizeof(SCSI::inquiry_t));
            _csw.dCSWDataResidue = cbw->dCBWDataTransferLength -
                                   sizeof(SCSI::inquiry_response_t);
            // Send the response
            while(_ep_in->is_active()) ;
            _ep_in->start_transfer((uint8_t *)&_inquiry_response,
                                   sizeof(SCSI::inquiry_response_t));
            break;
        }
        case SCSI::scsi_cmd_t::MODE_SENSE_6: {
            TUPP_LOG(LOG_INFO, "SCSI: MODE_SENSE_6");
            assert(cbw->bCBWCBLength == sizeof(SCSI::mode_sense_6_t));
            _mode_sense_6_response.mode_data_length = 3;
            _mode_sense_6_response.medium_type      = 0;
            _mode_sense_6_response.write_protect    = 0;
            _mode_sense_6_response.block_descriptor_length = 0;
            // Send the response
            while(_ep_in->is_active()) ;
            _ep_in->start_transfer((uint8_t *)&_mode_sense_6_response,
                                   sizeof(SCSI::mode_sense_6_response_t));
            break;
        }
        case SCSI::scsi_cmd_t::START_STOP_UNIT: {
            TUPP_LOG(LOG_INFO, "SCSI: START_STOP_UNIT");
            break;
        }
        case SCSI::scsi_cmd_t::PREVENT_ALLOW_MEDIUM_REMOVAL: {
            TUPP_LOG(LOG_INFO, "SCSI: PREVENT_ALLOW_MEDIUM_REMOVAL");
            break;
        }
        case SCSI::scsi_cmd_t::READ_CAPACITY_10: {
            assert(cbw->bCBWCBLength == sizeof(SCSI::read_capacity_10_t));
            _csw.dCSWDataResidue = cbw->dCBWDataTransferLength -
                                   sizeof(SCSI::read_capacity_10_response_t);
            uint16_t block_size  = 0;
            uint32_t block_count = 0;
            capacity_handler(block_size, block_count);
            assert(block_size == TUPP_MSC_BLOCK_SIZE);
            _read_capacity_10_response.logical_block_address = __htonl(block_count-1);
            _read_capacity_10_response.block_length          = __htonl(block_size);
            TUPP_LOG(LOG_INFO, "SCSI: READ_CAPACITY_10 (block size:%d blocks:%d)",
                     block_size, block_count);
            // Send the response
            while(_ep_in->is_active()) ;
            _ep_in->start_transfer((uint8_t *)&_read_capacity_10_response,
                                   sizeof(SCSI::read_capacity_10_response_t));
            break;
        }
        case SCSI::scsi_cmd_t::READ_FORMAT_CAPACITIES: {
            TUPP_LOG(LOG_INFO, "SCSI: READ_FORMAT_CAPACITIES");
            assert(cbw->bCBWCBLength == sizeof(SCSI::read_format_capacity_10_t));
            _csw.dCSWDataResidue = cbw->dCBWDataTransferLength -
                                   sizeof(SCSI::read_format_capacity_10_response_t);
            uint16_t block_size  = 0;
            uint32_t block_count = 0;
            capacity_handler(block_size, block_count);
            assert(block_size == TUPP_MSC_BLOCK_SIZE);
            _read_format_capacity_10_response.list_length     = 8;
            _read_format_capacity_10_response.descriptor_type = 2;
            _read_format_capacity_10_response.block_size_u16  = block_size;
            _read_format_capacity_10_response.block_num       = block_count;
            // Send the response
            while(_ep_in->is_active()) ;
            _ep_in->start_transfer((uint8_t *)&_read_format_capacity_10_response,
                                   sizeof(SCSI::read_format_capacity_10_response_t));
            break;
        }
        case SCSI::scsi_cmd_t::READ_10: {
            activity_handler(true, false);
            assert(cbw->bCBWCBLength == sizeof(SCSI::read_10_t));
            auto * read_cmd = (SCSI::read_10_t *)&cbw->CBWCB;
            _blocks_to_transfer = __ntohs(read_cmd->transfer_length);
            _blocks_transferred = 0;
            _block_addr = __ntohl(read_cmd->logical_block_address);
            _state = state_t::DATA_READ;
            TUPP_LOG(LOG_INFO, "SCSI: READ_10 (%d blocks)", _blocks_to_transfer);
            break;
        }
        case SCSI::scsi_cmd_t::WRITE_10: {
            _buffer_out_len = 0;
            activity_handler(false, true);
            assert(cbw->bCBWCBLength == sizeof(SCSI::write_10_t));
            auto * write_cmd = (SCSI::read_10_t *)&cbw->CBWCB;
            _blocks_to_transfer = __ntohs(write_cmd->transfer_length);
            _blocks_transferred = 0;
            _block_addr = __ntohl(write_cmd->logical_block_address);
            _state = state_t::DATA_WRITE;
            TUPP_LOG(LOG_INFO, "SCSI: WRITE_10 (%d blocks)", _blocks_to_transfer);
            // Let the next block arrive
            _buffer_out_len = 0;
            _ep_out->send_NAK(false);
            break;
        }
        default: {
            TUPP_LOG(LOG_ERROR, "Unrecognized SCSI command:");
            TUPP_LOG(LOG_ERROR, "sig: %x", cbw->dCBWSignature);
            TUPP_LOG(LOG_ERROR, "tag: %x", cbw->dCBWTag);
            TUPP_LOG(LOG_ERROR, "len: %x", cbw->dCBWDataTransferLength);
            TUPP_LOG(LOG_ERROR, "dir: %d", cbw->direction);
            TUPP_LOG(LOG_ERROR, "lun: %d", cbw->bCBWLUN);
            TUPP_LOG(LOG_ERROR, "cb len: %d", cbw->bCBWCBLength);
            for(int i=0; i < cbw->bCBWCBLength; ++i) {
                TUPP_LOG(LOG_ERROR, "  %x", cbw->CBWCB[i]);
            }
            break;
        }
    }
}
