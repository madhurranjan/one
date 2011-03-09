/* -------------------------------------------------------------------------- */
/* Copyright 2002-2010, OpenNebula Project Leads (OpenNebula.org)             */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#ifndef IMAGE_POOL_H_
#define IMAGE_POOL_H_

#include "PoolSQL.h"
#include "Image.h"
#include "NebulaLog.h"

#include <time.h>
#include <sstream>

#include <iostream>
#include <vector>

class AuthRequest;

using namespace std;

/**
 *  The Image Pool class.
 */
class ImagePool : public PoolSQL
{
public:

    ImagePool(SqlDB *       db,
              const string& _source_prefix,
              const string& _default_type,
              const string& _default_dev_prefix);

    ~ImagePool(){};

    /**
     *  Function to allocate a new Image object
     *    @param uid the user id of the image's owner
     *    @param stemplate template associated with the image
     *    @param oid the id assigned to the Image
     *    @return the oid assigned to the object,
     *                  -1 in case of failure
     *                  -2 in case of template parse failure
     */
    int allocate (
        int             uid,
        string          user_name,
        ImageTemplate * img_template,
        int *           oid,
        string&         error_str);

    /**
     **  Function to get a Image from the pool, if the object is not in memory
     *  it is loaded from the DB
     *    @param oid Image unique id
     *    @param lock locks the Image mutex
     *    @return a pointer to the Image, 0 if the Image could not be loaded
     */
    Image * get(int oid, bool lock)
    {
        return static_cast<Image *>(PoolSQL::get(oid,lock));
    };

    /**
     *  Function to get an Image from the pool using the image name
     *    @param name of the image
     *    @param lock locks the User mutex
     *    @return a pointer to the Image, 0 if the image could not be loaded
     */
    Image * get(const string&  name, int uid, bool lock)
    {
        return static_cast<Image *>(PoolSQL::get(name,uid,lock));
    }

    /** 
     *  Update a particular Image
     *    @param image pointer to Image
     *    @return 0 on success
     */
    int update(Image * image)
    {
        return image->update(db);
    };

    /** Drops an image from the DB, the image mutex MUST BE locked
     *    @param image pointer to Image
     *    @return 0 on success
     */
    int drop(Image * image)
    {
        return PoolSQL::drop(image);
    };

    /**
     *  Bootstraps the database table(s) associated to the Image pool
     */
    static void bootstrap(SqlDB *_db)
    {
        Image::bootstrap(_db);
    };

    /**
     *  Dumps the Image pool in XML format. A filter can be also added to the
     *  query
     *  @param oss the output stream to dump the pool contents
     *  @param where filter for the objects, defaults to all
     *  @return 0 on success
     */
    int dump(ostringstream& oss, const string& where)
    {
        return PoolSQL::dump(oss, "IMAGE_POOL", Image::table, where);
    }

    /**
     *  Generates a DISK attribute for VM templates using the Image metadata
     *    @param disk the disk to be generated
     *    @param disk_id the id for this disk
     *    @param index number of datablock images used by the same VM. Will be
     *                 automatically increased.
     *    @param img_type will be set to the used image's type
     *    @param uid of VM owner (to look for the image id within its images)
     *    @return 0 on success, -1 error, -2 not using the pool
     */
    int disk_attribute(VectorAttribute *  disk,
                       int                disk_id,
                       int *              index,
                       Image::ImageType * img_type,
                       int                uid);
    /**
     *  Generates an Authorization token for the DISK attribute
     *    @param disk the disk to be authorized
     *    @param uid of owner (to look for the image id within her images)
     *    @param ar the AuthRequest
     */
    void authorize_disk(VectorAttribute * disk, int uid, AuthRequest * ar);

    static const string& source_prefix()
    {
        return _source_prefix;
    };

    static const string& default_type()
    {
        return _default_type;
    };

    static const string& default_dev_prefix()
    {
        return _default_dev_prefix;
    };

private:
    //--------------------------------------------------------------------------
    // Configuration Attributes for Images
    // -------------------------------------------------------------------------
    /**
     * Path to the image repository
     **/
    static string  _source_prefix;

    /**
     * Default image type
     **/
    static string  _default_type;

    /**
     * Default device prefix
     **/
    static string  _default_dev_prefix;

    //--------------------------------------------------------------------------
    // Pool Attributes
    // -------------------------------------------------------------------------
    /**
     *  Factory method to produce Image objects
     *    @return a pointer to the new Image
     */
    PoolObjectSQL * create()
    {
        return new Image(-1,"",0);
    };
};

#endif /*IMAGE_POOL_H_*/
