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



#include "InstallerLiveCD.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>

namespace cloudos {
namespace system {
  
  InstallerLiveCD::InstallerLiveCD ( const std::string& p_root_dir ) : InstallerExtended ( p_root_dir ) {
    init();
  }
  
  InstallerLiveCD::InstallerLiveCD ( const fs::path& p_root_dir ) : InstallerExtended ( p_root_dir ) {
    init();
  }
  
  InstallerLiveCD::InstallerLiveCD ( std::string && p_root_dir ) : InstallerExtended ( std::move(p_root_dir) ) {
    init();
  }
  
  InstallerLiveCD::InstallerLiveCD ( fs::path && p_root_dir ) : InstallerExtended ( std::move(p_root_dir) ) {
    init();
  }
  
  const fs::path& InstallerLiveCD::getTarFilePath() const {
    return c_tar_file;
  }
  
  // 
  // P R O T E C T E D
  // 
  bool InstallerLiveCD::stepInstallLiveCDPackages() {
    if( searchForRPMsInDirectory("installer") == false ) {
      LOG_E() << "installer RPMs are required! Abort!";
      return false;
    }
    
    return installPackages();
  }
  
  bool InstallerLiveCD::stepCleanUpSetup() {
    // FIXME: clean up packages and remove some of these clean ups here...
    if( InstallerExtended::stepCleanUpSetup() == false ) {
      LOG_E() << "InstallerExtended clean up process failed... Abort!";
      return false;
    }
    
    fs::remove_all( getRootDirAsPath() / "usr/share/gtk-doc" );
    fs::remove_all( getRootDirAsPath() / "usr/lib/python2.7/test" );
    fs::remove_all( getRootDirAsPath() / "usr/lib/python2.7/debug" );
    fs::remove_all( getRootDirAsPath() / "usr/share/vim/vim73/spell/he.vim" );
    fs::remove_all( getRootDirAsPath() / "usr/share/vim/vim73/spell/yi.vim" );
    
    std::vector<std::string> to_be_killed;
    
    // $ROOT_DIR/usr/share/vim/vim73/lang/{af,ca,cs,de,eo,es,fi,fr,ga,it,ja,ko}*
    to_be_killed = boost::assign::list_of("af")("ca")("cs")("de")("eo")("es")("fi")("fr")("ga")("it")("ja")("ko").convert_to_container<std::vector<std::string> >();
    removeFiles( std::move(to_be_killed), getRootDirAsPath() / "usr/share/vim/vim73/lang", LOOKUP_BY_FULLMATCH );
    
    // $ROOT_DIR/usr/share/locale/e{l,o,s,t,u}*
    to_be_killed = boost::assign::list_of("el")("eo")("es")("et")("eu").convert_to_container<std::vector<std::string> >();
    removeFiles( std::move(to_be_killed), getRootDirAsPath() / "usr/share/locale", LOOKUP_BY_FULLMATCH );
    
    // $ROOT_DIR/usr/share/locale/[a-cf-z]*
    to_be_killed = std::vector<std::string>();
    to_be_killed.push_back("^([a-cf-z].*)");
    removeFiles( std::move(to_be_killed), getRootDirAsPath() / "usr/share/locale", LOOKUP_BY_REGEX );
    
    // $ROOT_DIR/usr/share/vim/vim73/lang/menu_[a-df-z]*
    to_be_killed = std::vector<std::string>();
    to_be_killed.push_back("^(menu_[a-df-z].*)");
    removeFiles( std::move(to_be_killed), getRootDirAsPath() / "usr/share/locale", LOOKUP_BY_REGEX );
    
    // $ROOT_DIR/usr/lib/*.a
    to_be_killed = std::vector<std::string>();
    to_be_killed.push_back(".a");
    removeFiles( std::move(to_be_killed), getRootDirAsPath() / "usr/lib", LOOKUP_BY_ENDSWITH );
    
    // $ROOT_DIR/usr/share/vim/vim73/doc/*.txt
    to_be_killed = std::vector<std::string>();
    to_be_killed.push_back(".txt");
    removeFiles( std::move(to_be_killed), getRootDirAsPath() / "usr/lib", LOOKUP_BY_ENDSWITH );
    
    // $ROOT_DIR/var/cache/zypper/RPMS/*.rpm
    to_be_killed = std::vector<std::string>();
    to_be_killed.push_back(".rpm");
    removeFiles( std::move(to_be_killed), getRootDirAsPath() / "var/cache/zypper/RPMS", LOOKUP_BY_ENDSWITH );
    
    return true;
  }
  
  bool InstallerLiveCD::stepCopyBasesystemRpms() {
    File from(   "/usr/share/cloudos/installer/basesystem-rpms.tar"                     );
    fs::path to( getRootDirAsPath() / "usr/share/cloudos/installer/basesystem-rpms.tar" );
    
    if( from.isAccessible() == false ) {
      LOG_E() << from.getFileName() << " is not available... Abort!";
      return false;
    }
    
    boost::system::error_code ec;
    fs::copy_file(from.getFileNameAsPath(), to, ec);
    if( ec ) {
      LOG_E() << "copy file failed... " << ec.message() << " ...Abort!";
      return false;
    }
    return true;
  }
  
  bool InstallerLiveCD::stepCreateTarFile() {
    // get image version:
    Command rpm("rpm");
    rpm.setChRootController(c_chroot);
    rpm << "-q" << "--queryformat" << "%{VERSION}-%{RELEASE}" << "cloudos-installer";
    if( rpm.waitUntilFinished() != 0 ) {
      LOG_E() << "could not determine image version and release number... Abort!";
      return false;
    }
    std::string version( rpm.getStdoutOutput().str() );
    boost::trim(version);
    
    if( c_chroot->disable() == false ) {
      LOG_E() << "failed to disable chroot env... Abort!";
      return false;
    }
    
    Directory root(getRootDirAsPath());
    if( root.isAccessible() == false ) {
      LOG_E() << "root dir is not accessible... Abort!";
      return false;
    }
    
    std::vector<fs::path> root_files;
    if( root.findFiles(root_files, ".*", LOOKUP_BY_REGEX, FILE_TYPE_ALL) == false ) {
      LOG_E() << "no files found within our root dir... Abort!";
      return false;
    }
    
    c_tar_file = "/tmp/installer-livecd-"+version+".tar.gz";
    
    if( fs::exists(c_tar_file) ) {
      boost::system::error_code ec;
      LOG_W() << c_tar_file.string() << " already exists! Removing the old one...";
      fs::remove( c_tar_file, ec );
      if( fs::exists(c_tar_file) ) {
        LOG_E() << "could not remove the old image data!" << ec.message() << " ...Abort!";
        return false;
      }
      LOG_W() << "old image data removed! SUCCESS!";
    }
    
    Command tar("tar");
    tar << "--transform" << "s,^"+getRootDir()+",.,S"; // remove leading dirs with ".", while creating the tar file...
    tar << "--absolute-names"; // don't remove leading / as we need it for --transform
    tar << "-czf" << c_tar_file.string();
    for(fs::path& file : root_files) {
      tar.addArgument( std::move(file.string()) );
    }
    
    return tar.waitUntilFinished() == 0;
  }
  
  bool InstallerLiveCD::stepCleanUpEnvironment() {
    if( c_chroot->disable() == false ) {
      LOG_E() << "failed to disable chroot... Abort!";
      return false;
    }
    
    LOG_I() << "removing root dir files...";
    fs::remove_all( getRootDirAsPath() );
    return true;
  }
  
  void InstallerLiveCD::init() {
    LOG_I() << "loading steps...";
    appendInstallerStep( "prepare_coredata", boost::bind(&InstallerLiveCD::stepPrepareCoreSystem,     this) );
    appendInstallerStep( "package_install",  boost::bind(&InstallerLiveCD::stepInstallLiveCDPackages, this) );
    appendInstallerStep( "cleanup",          boost::bind(&InstallerLiveCD::stepCleanUpSetup,          this) );
    appendInstallerStep( "copy_baserpms",    boost::bind(&InstallerLiveCD::stepCopyBasesystemRpms,    this) );
    appendInstallerStep( "create_tar_file",  boost::bind(&InstallerLiveCD::stepCreateTarFile,         this) );
    appendInstallerStep( "cleanup_env",      boost::bind(&InstallerLiveCD::stepCleanUpEnvironment,    this) );
    LOG_I() << "steps loaded";
  }
  
  // 
  // P R I V A T E
  // 
  void InstallerLiveCD::removeFiles ( std::vector< std::string > && p_elements, fs::path&& p_prepend_path, FileLookupMode p_mode ) {
    for(auto& element : p_elements) {
      LOG_I() << "try to remove files in " << p_prepend_path.string() << " and prase: " << element;
      if( fs::exists( p_prepend_path ) == false ) {
        LOG_I() << "does not exists";
        continue;
      }
      
      Directory dir( p_prepend_path );
      if( dir.isAccessible() == false ) {
        LOG_I() << "is not accessible";
        continue;
      }
      
      std::vector<fs::path> results;
      if( dir.findFiles(results, element, p_mode, FILE_TYPE_ALL) == false ) { // no results found
        LOG_I() << "no files found";
        continue;
      }
      
      for(auto& rm_element : results) {
        LOG_D() << "removing all from " << rm_element.string();
        fs::remove_all( rm_element );
      }
    }
  }
  
}}
