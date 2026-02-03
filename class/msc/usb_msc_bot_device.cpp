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
// (c) A. Terstegge  (Andreas.Terstegge@gmail.com)
//
// This file implements a Mass Storage Class (MSC) device
// in bulk-only-transfer (BOT) mode.
//
#include <cassert>
#include <cstring>
#include <machine/endian.h>

#include "usb_msc_bot_device.h"
#include "usb_structs.h"
#include "usb_log.h"

using namespace TUPP;
using enum TUPP::bInterfaceClass_t;
using enum TUPP::bInterfaceSubClass_t;
using enum TUPP::bInterfaceProtocol_t;
using enum TUPP::ep_attributes_t;
using enum TUPP::direction_t;
using enum SCSI::peripheral_device_type_t;
using enum SCSI::peripheral_qualifier_type_t;
using enum usb_log::log_level;

usb_msc_bot_device::usb_msc_bot_device(
        usb_device_controller & controller,
        usb_configuration     & configuration)
:  _configuration(configuration), _state(state_t::RECEIVE_CBW)
{
    // USB interface descriptor config
    _interface.set_bInterfaceClass   (IF_CLASS_MSC);
    _interface.set_bInterfaceSubClass(IF_SUBCLASS_SCSI_TRANSPARENT);
    _interface.set_bInterfaceProtocol(IF_PROTOCOL_MSC_BOT);

    // USB endpoints
    _ep_in  = controller.create_endpoint(_interface, DIR_IN,  TRANS_BULK);
    _ep_out = controller.create_endpoint(_interface, DIR_OUT, TRANS_BULK);

    // Prepare new request to receive data
    // Note: We can NOT receive larger blocks than TUPP_MSC_BLOCK_SIZE here,
    // because a partial USB packet will not be detected. If we e.g. ask for
    // 1024 bytes, but the device answers only with 512 bytes, we are at a
    // clean 64-byte border and would wait for more packets, which would
    // never be received.
    _ep_out->start_transfer(_buffer_out, TUPP_MSC_BLOCK_SIZE);

    // Endpoint handler
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

    // Handler for MSC specific requests
    _interface.setup_handler = [&](TUPP::setup_packet_t *pkt) {
        switch(pkt->bRequest) {
            case bRequest_t::REQ_MSC_BOT_RESET: {
                TUPP_LOG(LOG_INFO, "REQ_MSC_BOT_RESET");
                assert(pkt->wValue  == 0);
                assert(pkt->wLength == 0);
                // Continue with next CBW. According to the BOT
                // specification, the STALL status and the toggle
                // bits of the bulk EPs should not be touched. So
                // we do NOT reset the bulk EPs here ...
                _state = state_t::RECEIVE_CBW;
                break;
            }
            case bRequest_t::REQ_MSC_GET_MAX_LUN: {
                TUPP_LOG(LOG_INFO, "REQ_MSC_GET_MAX_LUN");
                assert(pkt->wValue  == 0);
                assert(pkt->wLength == 1);
                // Return the maximum LUN index. For devices not
                // supporting LUN numbers, a 0 should be returned.
                controller._ep0_in->start_transfer(&_max_lun, 1);
                break;
            }
            default: {
                TUPP_LOG(LOG_ERROR, "Unsupported MSC command 0x%x", pkt->bRequest);
            }
        }
    };

    // Initialize SCSI INQUIRY response
    _inquiry_response.peripheral_device     = SBC_4_DIRECT_ACCESS;
    _inquiry_response.peripheral_qualifier  = DEVICE_CONNECTED_TO_LUN;
    _inquiry_response.removable_media       = 1;
    _inquiry_response.version               = SCSI::version_t::NO_STANDARD;
    _inquiry_response.response_data_format  = 2;
    _inquiry_response.additional_length     = sizeof(SCSI::inquiry_response_t) - 5;

    // Initialize the SCSI SENSE response
    _sense_fixed_response.response_code     = SCSI::response_code_t::CURRENT_ERROR;
    _sense_fixed_response.valid             = 1;
    _sense_fixed_response.sense_key         = SCSI::sense_key_t::NO_SENSE;
    _sense_fixed_response.add_sense_len     = sizeof (_sense_fixed_response) - 8;

    // Initialize the SCSI MODE SENSE 6 response
    _mode_sense_6_response.mode_data_length = sizeof(_mode_sense_6_response)-1;
    _mode_sense_6_response.medium_type      = 0;
    _mode_sense_6_response.write_protect    = false;
    _mode_sense_6_response.block_descriptor_length = 0;
}



void usb_msc_bot_device::scsi_success() {
    // Set sense keys
    _sense_fixed_response.sense_key         = SCSI::sense_key_t::NO_SENSE;
    _sense_fixed_response.add_sense_code    = 0;
    _sense_fixed_response.add_sense_qualifier = 0;
    // Set Command Status Wrapper
    _csw.dCSWDataResidue                    = 0;
    _csw.bCSWStatus                         = MSC::csw_status_t::CMD_PASSED;
}

void usb_msc_bot_device::scsi_fail(SCSI::sense_key_t key, uint8_t code, uint8_t qualifier) {
    // Set sense keys
    _sense_fixed_response.sense_key         = key;
    _sense_fixed_response.add_sense_code    = code;
    _sense_fixed_response.add_sense_qualifier = qualifier;
    // Set Command Status Wrapper
    _csw.dCSWDataResidue                    = 0;
    _csw.bCSWStatus                         = MSC::csw_status_t::CMD_FAILED;
}


// This method implements a simple state machine
// according to the MSC BOT specification. This
// method has to be called by the user program
// in a tight loop (also in e.g. a RTOS thread).
void usb_msc_bot_device::handle_request() {
    switch(_state) {
        case state_t::RECEIVE_CBW: {
            if (_buffer_out_len == 0) {
                // No data received? -> stay in this state and wait...
                break;
            }
            TUPP_LOG(LOG_DEBUG, "STATE: RECEIVE_CBW");
            // set pointer to CBW
            auto *cbw = (MSC::cbw_t *) _buffer_out;

            // Check the CBW. Is it valid ?
            if ((_buffer_out_len != sizeof(MSC::cbw_t)) ||
                (cbw->dCBWSignature != MSC::cbw_signature)) {
                // Error class 6.6.1: Stall bulk endpoints.
                // Stay in the RECEIVE_CBW state!
                _ep_in->send_stall(true);
                _ep_out->send_stall(true);
                // Signal that data has been processed
                _buffer_out_len = 0;
                // Let new data flow in
                _ep_out->send_NAK(false);
                return;
            }

            // Prepare the next command status wrapper.
            // Default is CMD_PASSED -> no error
            _csw.dCSWSignature   = MSC::csw_signature;
            _csw.dCSWTag         = cbw->dCBWTag;
            _csw.dCSWDataResidue = 0;
            _csw.bCSWStatus      = MSC::csw_status_t::CMD_PASSED;

            // Default: Continue with sending the CSW
            _state = state_t::SEND_CSW;

            // Handle the received SCSI command.
            process_scsi_command();

            // Mark data as consumed and accept new packets
            _buffer_out_len = 0;
            _ep_out->send_NAK(false);
            break;
        }
        case state_t::SEND_CSW: {
            if (_ep_in->is_active()) {
                // EP still active, so keep this state
                // and wait for the EP to be usable...
                break;
            }
            TUPP_LOG(LOG_DEBUG, "STATE: SEND_CSW");
            _ep_in->start_transfer((uint8_t *)&_csw, sizeof(_csw));
            // Continue with next CBW
            _state = state_t::RECEIVE_CBW;
            break;
        }
        case state_t::DATA_READ: {
            if (_ep_in->is_active()) {
                // EP still active, so keep this state
                // and wait for the EP to be usable...
                break;
            }
            TUPP_LOG(LOG_DEBUG, "STATE: DATA_READ");
            // Read data from device
            uint8_t res = read_handler(_buffer_in, _block_addr++);
            _ep_in->start_transfer(_buffer_in, TUPP_MSC_BLOCK_SIZE);
            _blocks_transferred++;
            // Check if all blocks have been received and
            // we have to leave this state
            if (_blocks_transferred == _blocks_to_transfer) {
                _state = state_t::SEND_CSW;
            }
            if (res) {
                scsi_fail(SCSI::sense_key_t::NOT_READY, 0x3a, 0);
            }
            break;
        }
        case state_t::DATA_WRITE: {
            if (_buffer_out_len == 0) {
                // No data received? -> keep this state and wait...
                break;
            }
            TUPP_LOG(LOG_DEBUG, "STATE: DATA_WRITE");
            assert(_buffer_out_len == TUPP_MSC_BLOCK_SIZE);
            // Write data to device
            uint8_t res = write_handler(_buffer_out, _block_addr++);
            _blocks_transferred++;
            // Check if all blocks have been written and
            // we have to leave this state
            if (_blocks_transferred == _blocks_to_transfer) {
                _state = state_t::SEND_CSW;
            }
            if (res) {
                scsi_fail(SCSI::sense_key_t::NOT_READY, 0x3a, 0);
            }
            // Mark data as consumed and accept new packets
            _buffer_out_len = 0;
            _ep_out->send_NAK(false);
            break;
        }
    }
}

void usb_msc_bot_device::process_scsi_command() {
    TUPP_LOG(LOG_DEBUG, "process_scsi_command()");
    auto *  cbw = (MSC::cbw_t *) _buffer_out;
    auto    cmd = (SCSI::scsi_cmd_t)cbw->CBWCB[0];

    uint8_t * response_data;
    size_t    response_len = 0;
    size_t    response_len_expected = cbw->dCBWDataTransferLength;

    switch(cmd) {
        // This command does not return any data,
        // but internally sets the SENSE reply.
        case SCSI::scsi_cmd_t::TEST_UNIT_READY: {
            TUPP_LOG(LOG_INFO, "SCSI: TEST_UNIT_READY");
            assert(cbw->bCBWCBLength == sizeof(SCSI::test_unit_ready_t));
            if (_device_ready) {
                _sense_fixed_response.sense_key           = SCSI::sense_key_t::NO_SENSE;
                _sense_fixed_response.add_sense_code      = 0;
                _sense_fixed_response.add_sense_qualifier = 0;
            } else {
                _sense_fixed_response.sense_key           = SCSI::sense_key_t::NOT_READY;
                _sense_fixed_response.add_sense_code      = 4;
                _sense_fixed_response.add_sense_qualifier = 0;
            }
            break;
        }
        case SCSI::scsi_cmd_t::REQUEST_SENSE: {
            TUPP_LOG(LOG_INFO, "SCSI: REQUEST_SENSE");
            assert(cbw->bCBWCBLength == sizeof(SCSI::request_sense_t));
            // Set response
            response_data = (uint8_t *)&_sense_fixed_response;
            response_len  = sizeof(SCSI::request_sense_fixed_response_t);
            break;
        }
        case SCSI::scsi_cmd_t::INQUIRY: {
            TUPP_LOG(LOG_INFO, "SCSI: INQUIRY");
            //assert(cbw->bCBWCBLength == sizeof(SCSI::inquiry_t));
            _csw.dCSWDataResidue = cbw->dCBWDataTransferLength -
                                   sizeof(SCSI::inquiry_response_t);
            // Set response
            response_data = (uint8_t *)&_inquiry_response;
            response_len  = sizeof(SCSI::inquiry_response_t);
            break;
        }
        case SCSI::scsi_cmd_t::MODE_SENSE_6: {
            TUPP_LOG(LOG_INFO, "SCSI: MODE_SENSE_6");
            assert(cbw->bCBWCBLength == sizeof(SCSI::mode_sense_6_t));
            _csw.dCSWDataResidue = cbw->dCBWDataTransferLength -
                                   sizeof(SCSI::mode_sense_6_response_t);
            bool write_protect = false;
            if (is_writeable_handler) {
                write_protect = !is_writeable_handler();
            }
            // Set response
            _mode_sense_6_response.write_protect = write_protect;
            response_data = (uint8_t *)&_mode_sense_6_response;
            response_len  = sizeof(SCSI::mode_sense_6_response_t);
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            break;
        }
        case SCSI::scsi_cmd_t::START_STOP_UNIT: {
            TUPP_LOG(LOG_INFO, "SCSI: START_STOP_UNIT");
            assert(cbw->bCBWCBLength == sizeof(SCSI::start_stop_unit_t));
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            auto * ssu = (SCSI::start_stop_unit_t *)cbw->CBWCB;

            TUPP_LOG(LOG_INFO, "%d %d %d", ssu->power_condition, ssu->start, ssu->loej);

            if (!ssu->start && ssu->loej) {
                //_device_ready = false;
            }
            if (start_stop_handler) {
                start_stop_handler(ssu->power_condition, ssu->start, ssu->loej);
            }
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            break;
        }
        case SCSI::scsi_cmd_t::PREVENT_ALLOW_MEDIUM_REMOVAL: {
            TUPP_LOG(LOG_INFO, "SCSI: PREVENT_ALLOW_MEDIUM_REMOVAL");
            assert(cbw->bCBWCBLength == sizeof(SCSI::prevent_allow_media_removal_t));
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            auto * pamr = (SCSI::prevent_allow_media_removal_t *)cbw->CBWCB;
            if (remove_handler) {
                remove_handler(pamr->prevent);
            }
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            break;
        }
        case SCSI::scsi_cmd_t::READ_CAPACITY_10: {
            assert(cbw->bCBWCBLength == sizeof(SCSI::read_capacity_10_t));
            assert(cbw->dCBWDataTransferLength == sizeof(SCSI::read_capacity_10_response_t));
            _csw.dCSWDataResidue = cbw->dCBWDataTransferLength -
                                   sizeof(SCSI::read_capacity_10_response_t);
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            uint16_t block_size  = 0;
            uint32_t block_count = 0;
            capacity_handler(block_size, block_count);
            assert(block_size == TUPP_MSC_BLOCK_SIZE);
            _read_capacity_10_response.logical_block_address = __htonl(block_count-1);
            _read_capacity_10_response.block_length          = __htonl(block_size);
            TUPP_LOG(LOG_INFO, "SCSI: READ_CAPACITY_10 (block size:%d blocks:%d)",
                     block_size, block_count);
            // Set response
            response_data = (uint8_t *)&_read_capacity_10_response;
            response_len  = sizeof(SCSI::read_capacity_10_response_t);
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            break;
        }
        case SCSI::scsi_cmd_t::READ_FORMAT_CAPACITIES: {
            TUPP_LOG(LOG_INFO, "SCSI: READ_FORMAT_CAPACITIES");
            assert(cbw->bCBWCBLength == sizeof(SCSI::read_format_capacity_t));
            _csw.dCSWDataResidue = cbw->dCBWDataTransferLength -
                                   sizeof(SCSI::read_format_capacity_10_response_t);
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            uint16_t block_size  = 0;
            uint32_t block_count = 0;
            capacity_handler(block_size, block_count);
            assert(block_size == TUPP_MSC_BLOCK_SIZE);
            _read_format_capacity_10_response.list_length     = 8;
            _read_format_capacity_10_response.descriptor_type = 2;
            _read_format_capacity_10_response.block_size_u16  = block_size;
            _read_format_capacity_10_response.block_num       = block_count;
            // Set response
            response_data = (uint8_t *)&_read_format_capacity_10_response;
            response_len  = sizeof(SCSI::read_format_capacity_10_response_t);
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
            break;
        }
        case SCSI::scsi_cmd_t::READ_10: {
            assert(cbw->bCBWCBLength == sizeof(SCSI::read_10_t));
            auto * read_cmd = (SCSI::read_10_t *)&cbw->CBWCB;
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
                _sense_fixed_response.sense_key             = SCSI::sense_key_t::NOT_READY;
                _sense_fixed_response.add_sense_code        = 4;
                _sense_fixed_response.add_sense_qualifier   = 0;
//                break;
            }
            _blocks_to_transfer = __ntohs(read_cmd->transfer_length);
            _blocks_transferred = 0;
            _block_addr = __ntohl(read_cmd->logical_block_address);
            TUPP_LOG(LOG_INFO, "SCSI: READ_10 (%d blocks)", _blocks_to_transfer);
            _state = state_t::DATA_READ;
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
                _state = state_t::SEND_CSW;
            }
            break;
        }
        case SCSI::scsi_cmd_t::WRITE_10: {
            _buffer_out_len = 0;
            assert(cbw->bCBWCBLength == sizeof(SCSI::write_10_t));
            auto * write_cmd = (SCSI::write_10_t *)&cbw->CBWCB;
            // Check if we may write to this device
            bool write_protect = false; // Default is RW
            if (is_writeable_handler) {
                write_protect = !is_writeable_handler();
            }
            if (write_protect) {
                TUPP_LOG(LOG_WARNING, "SCSI: Write on write-protected device!");
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
                _sense_fixed_response.sense_key             = SCSI::sense_key_t::DATA_PROTECT;
                _sense_fixed_response.add_sense_code        = 0x27;
                _sense_fixed_response.add_sense_qualifier   = 0x00;
                // Let the next block arrive
                _buffer_out_len = 0;
                _ep_out->send_NAK(false);
                break;
            }
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
                _sense_fixed_response.sense_key             = SCSI::sense_key_t::NOT_READY;
                _sense_fixed_response.add_sense_code        = 4;
                _sense_fixed_response.add_sense_qualifier   = 0;
                // Let the next block arrive
                _buffer_out_len = 0;
                _ep_out->send_NAK(false);
//                break;
            }
            _blocks_to_transfer = __ntohs(write_cmd->transfer_length);
            _blocks_transferred = 0;
            _block_addr = __ntohl(write_cmd->logical_block_address);
            TUPP_LOG(LOG_INFO, "SCSI: WRITE_10 (%d blocks)", _blocks_to_transfer);
            _state = state_t::DATA_WRITE;
            // Let the next block arrive
            _buffer_out_len = 0;
            _ep_out->send_NAK(false);
            if (!_device_ready) {
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
                _state = state_t::SEND_CSW;
            }
            break;
        }
        default: {
            // Unimplemented SCSI command: Show the details
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

    // Analyze response situation (no DATA transfers)
    if (_state == state_t::SEND_CSW) {
        if (response_len_expected) {
            if (response_len) {
                if (response_len != response_len_expected) {
                    TUPP_LOG(LOG_WARNING, "Expected response len (%d) differs from response len (%d)",
                             response_len_expected, response_len);
                }
                // Do not send more data than expected
                if (response_len > response_len_expected) {
                    response_len = response_len_expected;
                }
                // Send the response
                while (_ep_in->is_active());
                _ep_in->start_transfer(response_data, response_len);
            } else {
                TUPP_LOG(LOG_WARNING, "SCSI response expected but no data");
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
        } else {
            // Case 6.7.1: Host did not expect a data transfer
            if (response_len) {
                TUPP_LOG(LOG_WARNING, "No SCSI response expected but data");
                _csw.bCSWStatus = MSC::csw_status_t::CMD_FAILED;
            }
        }
    }
}

void usb_msc_bot_device::set_vendor_id(const char * id) {
    TUPP_LOG(LOG_DEBUG, "set_vendor_id(%s)", id);
    if (strlen(id) > 8) {
        TUPP_LOG(LOG_WARNING, "SCSI Vendor ID too long. Truncated!");
    }
    strncpy((char*)_inquiry_response.vendor_id, id,
            sizeof _inquiry_response.vendor_id - 1);
}

void usb_msc_bot_device::set_product_id(const char * id) {
    TUPP_LOG(LOG_DEBUG, "set_product_id(%s)", id);
    if (strlen(id) > 16) {
        TUPP_LOG(LOG_WARNING, "SCSI Product ID too long. Truncated!");
    }
    strncpy((char*)_inquiry_response.product_id, id,
            sizeof _inquiry_response.product_id - 1);
}

void usb_msc_bot_device::set_product_rev(const char * rev) {
    TUPP_LOG(LOG_DEBUG, "set_product_rev(%s)", rev);
    if (strlen(rev) > 4) {
        TUPP_LOG(LOG_WARNING, "SCSI Product Rev too long. Truncated!");
    }
    strncpy((char*)_inquiry_response.product_rev, rev,
            sizeof _inquiry_response.product_rev - 1);
}

void usb_msc_bot_device::set_device_ready(bool ready) {
    _device_ready = ready;
}
