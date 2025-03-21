#!/usr/bin/env bash

file="$1"; shift
if [ ! -f "$file" ]; then
  echo must pass filename on command line.
  exit 3
fi

tempfile="$(mktemp "$TMP/zz_temp_codefix.XXXXXX")"

#echo temp file is $tempfile

cat "$file" \
  | sed -e 's/command_line::__arg/application::__arg/g' \
  | sed -e 's/IMPLEMENT_CLASS_NAME/DEFINE_CLASS_NAME/g' \
  | sed -e 's/istring/astring/g' \
  | sed -e 's/byte_format\([^t]\)/byte_formatter\1/g' \
  | sed -e 's/isprintf/a_sprintf/g' \
  | sed -e 's/portable::sleep_ms/time_control::sleep_ms/g' \
  | sed -e 's/portable::env_string/environment::get/g' \
  | sed -e 's/portable::launch_process/launch_process::run/g' \
  | sed -e 's/portable::application_name/application_configuration::application_name/g' \
  | sed -e 's/portable::process_id/application_configuration::process_id/g' \
  | sed -e 's/log_base::platform_ending/parser_bits::platform_eol_to_chars/g' \
  | sed -e 's/ithread/ethread/g' \
  | sed -e 's/timed_object/timeable/g' \
  | sed -e 's/utility::timestamp(/time_stamp::notarize(/g' \
  | sed -e 's/anchor_window/hoople_service/g' \
  | sed -e 's/basis::attach/structures::attach/g' \
  | sed -e 's/basis::detach/structures::detach/g' \
  | sed -e 's/portable::system_error/critical_events::system_error/g' \
  | sed -e 's/basis::pack\([^a]\)/structures::pack_array\1/g' \
  | sed -e 's/basis::unpack/structures::unpack_array/g' \
  | sed -e 's/<data_struct/<structures/g' \
  | sed -e 's/<basis\/set/<structures\/set/g' \
  | sed -e 's/basis::set/structures::set/g' \
  | sed -e 's/<basis\/object_base/<basis\/contracts/g' \
  | sed -e 's/object_base/root_object/g' \
  | sed -e 's/<basis\/function.h/<basis\/functions.h/g' \
  | sed -e 's/^#include <basis\/portable.h> *$//g' \
  | sed -e 's/^#include <basis\/log_base.h> *$//g' \
  | sed -e 's/^#include <basis\/utility.h> *$//g' \
  | sed -e 's/^#include <basis\/packable.h> *$//g' \
  | sed -e 's/^#include <basis\/auto_synch.h> *$//g' \
  | sed -e 's/class infoton_list;//g' \
  | sed -e 's/^#include "[_a-zA-Z0-9]*_dll.h" *$//g' \
  | sed -e 's/^#include "dll_[_a-zA-Z0-9]*.h" *$//g' \
  | sed -e 's/^#ifndef .*IMPLEMENTATION_FILE *$//g' \
  | sed -e 's/^#define .*IMPLEMENTATION_FILE *$//g' \
  | sed -e 's/^#endif .*IMPLEMENTATION_FILE *$//g' \
  | sed -e 's/convert_utf/utf_conversion/g' \
  | sed -e 's/mechanisms\/time_stamp/timely\/time_stamp/g' \
  | sed -e 's/mechanisms\/roller/structures\/roller/g' \
  | sed -e 's/mechanisms\/safe_roller/processes\/safe_roller/g' \
  | sed -e 's/basis.string_array/structures\/string_array/g' \
  | sed -e 's/opsystem.application_shell/application\/application_shell/g' \
  | sed -e 's/opsystem.filename/filesystem\/filename/g' \
  | sed -e 's/opsystem.heavy_file_ops/filesystem\/heavy_file_ops/g' \
  | sed -e 's/opsystem.huge_file/filesystem\/huge_file/g' \
  | sed -e 's/opsystem.application_base/application\/base_application/g' \
  | sed -e 's/opsystem.command_line/application\/command_line/g' \
  | sed -e 's/opsystem.directory/filesystem\/directory/g' \
  | sed -e 's/opsystem.rendezvous/application\/rendezvous/g' \
  | sed -e 's/opsystem.singleton_application/application\/singleton_application/g' \
  | sed -e 's/opsystem.timer_driver/timely\/timer_driver/g' \
  | sed -e 's/opsystem.ini_config/configuration\/ini_configurator/g' \
  | sed -e 's/opsystem.path_config/configuration\/application_config/g' \
  | sed -e 's/opsystem.byte_filer/filesystem\/byte_filer/g' \
  | sed -e 's/sockets.address/sockets\/internet_address/g' \
  | sed -e 's/path_configuration/application_configuration/g' \
  | sed -e 's/mechanisms.timer/timely\/stopwatch/g' \
  | sed -e 's/mechanisms.ethread/processes\/ethread/g' \
  | sed -e 's/mechanisms.safe_callback/processes\/safe_callback/g' \
  | sed -e 's/mechanisms.thread_cabinet/processes\/thread_cabinet/g' \
  | sed -e 's/basis.chaos/mathematics\/chaos/g' \
  | sed -e 's/[A-Z_][A-Z_]*CLASS_STYLE //g' \
  | sed -e 's/[A-Z_][A-Z_]*FUNCTION_STYLE //g' \
  | sed -e 's/\([^:]\)u_int/\1basis::u_int/g' \
  | sed -e 's/\([^:]\)u_short/\1basis::u_short/g' \
  | sed -e 's/class astring;/#include <basis\/astring.h>/g' \
  | sed -e 's/class int_set;/#include <structures\/set.h>/g' \
  | sed -e 's/class int_roller;/#include <structures\/roller.h>/g' \
  | sed -e 's/class outcome;/#include <basis\/outcome.h>/g' \
  | sed -e 's/class mutex;/#include <basis\/mutex.h>/g' \
  | sed -e 's/class ethread;/#include <processes\/ethread.h>/g' \
  | sed -e 's/class byte_filer;/#include <filesystem\/byte_filer.h>/g' \
  | sed -e 's/class string_array;/#include <structures\/string_array.h>/g' \
  | sed -e 's/class string_table;/#include <structures\/string_table.h>/g' \
  | sed -e 's/class byte_array;/#include <basis\/byte_array.h>/g' \
  | sed -e 's/class string_set;/#include <structures\/set.h>/g' \
  | sed -e 's/class time_stamp;/#include <timely\/time_stamp.h>/g' \
  | sed -e 's/class directory_tree;/#include <filesystem\/directory_tree.h>/g' \
  | sed -e 's/class filename_list;/#include <filesystem\/filename_list.h>/g' \
  | sed -e 's/class chaos;/#include <mathematics\/chaos.h>/g' \
  | sed -e 's/class configurator;/#include <configuration\/configurator.h>/g' \
  | sed -e 's/class unique_int;/#include <structures\/unique_id.h>/g' \
  | sed -e 's/class tcpip_stack;/#include <sockets\/tcpip_stack.h>/g' \
  | sed -e 's/class safe_roller;/#include <processes\/safe_roller.h>/g' \
  | sed -e 's/class blowfish_crypto;/#include <crypto\/blowfish_crypto.h>/g' \
  | sed -e 's/class RSA_crypto;/#include <crypto\/RSA_crypto.h>/g' \
  | sed -e 's/class entity_data_bin;/#include <octopus\/entity_data_bin.h>/g' \
  | sed -e 's/class infoton;/#include <octopus\/infoton.h>/g' \
  | sed -e 's/class octopus_request_id;/#include <octopus\/entity_defs.h>/g' \
  | sed -e 's/class internet_address;/#include <sockets\/internet_address.h>/g' \
  | sed -e 's/class machine_uid;/#include <sockets\/machine_uid.h>/g' \
  | sed -e 's/class spocket;/#include <sockets\/spocket.h>/g' \
  | sed -e 's/class encryption_tentacle;/#include <tentacles\/encryption_tentacle.h>/g' \
  | sed -e 's/class login_tentacle;/#include <tentacles\/login_tentacle.h>/g' \
  | sed -e 's/class thread_cabinet;/#include <processes\/thread_cabinet.h>/g' \
  | sed -e 's/RSA_crypto/rsa_crypto/g' \
  | sed -e 's/float_plus<double>/double_plus/g' \
  | sed -e 's/basis::obscure_/structures::obscure_/g' \
  | sed -e 's/program_wide_logger()/program_wide_logger::get()/g' \
  | sed -e 's/textual.tokenizer/configuration\/tokenizer/g' \
  | sed -e 's/\([^_]\)tokenizer/\1variable_tokenizer/g' \
  | sed -e 's/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/[\/]*/\/\/\/\/\/\/\/\/\/\/\/\/\/\//g' \
  >"$tempfile"

mv "$tempfile" "$file"


