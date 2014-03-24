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



#ifndef CLOUDOS_SYSTEM_INSTALLER_MANAGEMENT_SYSTEM_HPP__
#define CLOUDOS_SYSTEM_INSTALLER_MANAGEMENT_SYSTEM_HPP__

#include <vector>

#include <boost/container/stable_vector.hpp>

#include <cloudos/system/InstallerExtended.hpp>

namespace cloudos {
namespace system {
  
  class InstallerLiveCD;
  typedef boost::shared_ptr<InstallerLiveCD> InstallerLiveCDPointer;
  
  /**
   * Why we need this class?
   * This class is able to create a tar file for our live cd system
   * 
   * What is possible with this class?
   * Create an tar file of a bootstrapped and stripped cloud os system
   * 
   * Is there a special behaviour, on special circumstances?
   * Non known
   * 
   * How to use this class as an object?
   * InstallerLiveCD live("/tmp/path/to/install/");
   * live.setup();
   */
  class InstallerLiveCD : public InstallerExtended {
  public:
    /**
     * Every CoreSystem needs at least a base path, where it will be installed in.
     * This directory will be givven by p_root_dir
     */
    InstallerLiveCD ( const std::string& p_root_dir );
    
    /**
     * Every CoreSystem needs at least a base path, where it will be installed in.
     * This directory will be givven by p_root_dir
     */
    InstallerLiveCD ( const fs::path& p_root_dir );
    
    /**
     * Every CoreSystem needs at least a base path, where it will be installed in.
     * This directory will be givven by p_root_dir
     */
    InstallerLiveCD ( std::string && p_root_dir );
    
    /**
     * Every CoreSystem needs at least a base path, where it will be installed in.
     * This directory will be givven by p_root_dir
     */
    InstallerLiveCD ( fs::path && p_root_dir );
    
    /**
     * Returns the path to the tar file from the created live cd image.
     * 
     * You need to call setup() first, else the returned filepath will be empty!
     */
    const fs::path& getTarFilePath() const;
    
  protected:
    
    /**
     * Inits all class members...
     */
    virtual void init() override;
    
    /**
     * Calls zypper to install our packages
     */
    bool stepInstallLiveCDPackages();
    
    /**
     * Removes unwanted files from our tar file
     */
    virtual bool stepCleanUpSetup() override;
    
    /**
     * Copy file basesystem-rpms.tar to our live cd image
     */
    bool stepCopyBasesystemRpms();
    
    /**
     * Creates the tar file from our created image directory
     */
    bool stepCreateTarFile();
    
    /**
     * Removes our build dir (RootPath)
     */
    bool stepCleanUpEnvironment();
    
  private:
    fs::path c_tar_file;
    
    void removeFiles( std::vector<std::string>&& p_elements, fs::path&& p_prepend_path, FileLookupMode p_mode );
  };
}}

#endif
