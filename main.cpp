/*
 *   libcloudos
 *   Copyright (C) 2014  Tony Wolf <wolf@os-forge.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/



#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/thread.hpp>

#include <cloudos/system/Command.hpp>
#include "InstallerLiveCD.hpp"
#include <cloudos/core/logging.hpp>

typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend > sink_t;

namespace expr = boost::log::expressions;
namespace attr = boost::log::attributes;

using namespace cloudos;

int main(int argc, char **argv) {
  
  blog::add_common_attributes();
  blog::core::get()->add_global_attribute("TimeStamp", blog::attributes::local_clock());
  blog::core::get()->add_global_attribute("ThreadID", blog::attributes::current_thread_id());
  
  auto backend = boost::make_shared< boost::log::sinks::text_file_backend >( boost::log::keywords::file_name = "/tmp/installer_livecd_%5N.log" );
  boost::shared_ptr< sink_t > sink(new sink_t(backend));
  
  sink->set_formatter
        (
          expr::format("[%1%] [%2%] [%3%] %4%")
          % expr::attr< boost::posix_time::ptime >("TimeStamp")
          % expr::attr< boost::thread::id >("ThreadID")
          % expr::attr< blog::trivial::severity_level >("Severity")
          % expr::smessage
        );
  
  blog::core::get()->add_sink( sink );
  
  core::Logger::LogSourceType& lg = core::cloudos_log::get();
  
  // disable kernel messages on console
  system::Command dmesg("dmesg");
  dmesg << "--console-off";
  dmesg.waitUntilFinished();
  
  try {
    
    boost::shared_ptr<system::InstallerLiveCD> installer( new system::InstallerLiveCD( fs::path("/tmp/cloudos/livecd") ) );
    fs::path tar_file("/usr/share/cloudos/installer/basesystem-rpms.tar");
    installer->setPackagesTarFile( std::move(tar_file) );
    if( installer->setup() == false ) {
      std::cerr << "setup failed... please see the logfile: /tmp/installer_livecd_%5N.log" << std::endl;
    } else {
      std::cout << "ResultingTarFile: " << installer->getTarFilePath().string() << std::endl;
    }
    
  } catch(std::exception& e) {
    BOOST_LOG_SEV(lg, blog::trivial::fatal) << "UI, std exception occured: " << e.what();
    sink->flush();
  } catch(boost::system::error_code& ec) {
    BOOST_LOG_SEV(lg, blog::trivial::fatal) << "UI, boost exception occured: " << ec.message();
    sink->flush();
  } catch(...) {
    BOOST_LOG_SEV(lg, blog::trivial::fatal) << "UI, unknown exception occured :-(";
    sink->flush();
  }
  
  dmesg.clearArguments();
  dmesg << "--console-on";
  dmesg.waitUntilFinished();
  
  return 0;
}
