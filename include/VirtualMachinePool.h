/* -------------------------------------------------------------------------- */
/* Copyright 2002-2009, Distributed Systems Architecture Group, Universidad   */
/* Complutense de Madrid (dsa-research.org)                                   */
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

#ifndef VIRTUAL_MACHINE_POOL_H_
#define VIRTUAL_MACHINE_POOL_H_

#include "PoolSQL.h"
#include "VirtualMachine.h"

#include <time.h>

using namespace std;


/**
 *  The Virtual Machine Pool class. ...
 */
class VirtualMachinePool : public PoolSQL
{
public:

    VirtualMachinePool(SqliteDB * db):PoolSQL(db,VirtualMachine::table){};

    ~VirtualMachinePool(){};

    /**
     *  Function to allocate a new VM object
     *    @param uid user id (the owner of the VM)
     *    @param stemplate a string describing the VM
     *    @param oid the id assigned to the VM (output)
     *    @param on_hold flag to submit on hold
     *    @return 0 on success, -1 error inserting in DB or -2 error parsing
     *  the template
     */
    int allocate (
        int     uid,
        const  string& stemplate,
        int *  oid,
        bool   on_hold = false);

    /**
     *  Function to get a VM from the pool, if the object is not in memory
     *  it is loade from the DB
     *    @param oid VM unique id
     *    @param lock locks the VM mutex
     *    @return a pointer to the VM, 0 if the VM could not be loaded
     */
    VirtualMachine * get(
        int     oid,
        bool    lock)
    {
        return static_cast<VirtualMachine *>(PoolSQL::get(oid,lock));
    };

    /**
     *  Function to get the IDs of running VMs
     *   @param oids a vector that contains the IDs
     *   @return 0 on success
     */
    int get_running(
        vector<int>&    oids);

    /**
     *  Function to get the IDs of pending VMs
     *   @param oids a vector that contains the IDs
     *   @return 0 on success
     */
    int get_pending(
        vector<int>&    oids);

    //--------------------------------------------------------------------------
    // Virtual Machine DB access functions
    //--------------------------------------------------------------------------

    /**
     *  Updates the template of a VM, adding a new attribute (replacing it if
     *  already defined), the vm's mutex SHOULD be locked
     *    @param vm pointer to the virtual machine object
     *    @param name of the new attribute
     *    @param value of the new attribute
     *    @return 0 on success
     */
    int update_template_attribute(
        VirtualMachine *	vm,
        string&			 	name,
        string&			 	value)
    {
    	return vm->update_template_attribute(db,name,value);
    }

    /**
     *  Updates the history record of a VM, the vm's mutex SHOULD be locked
     *    @param vm pointer to the virtual machine object
     *    @return 0 on success
     */
    int update_history(
        VirtualMachine * vm)
    {
        return vm->update_history(db);
    }

    /**
     *  Updates the previous history record, the vm's mutex SHOULD be locked
     *    @param vm pointer to the virtual machine object
     *    @return 0 on success
     */
    int update_previous_history(
        VirtualMachine * vm)
    {
        return vm->update_previous_history(db);
    }

    /**
     *  Parse a string and substitute variables (e.g. $NAME) using template
     *  values:
     *    @param vm_id, ID of the VM used to substitute the variables
     *    @param attribute, the string to be parsed
     *    @param parsed, the resulting parsed string
     *    @param error_msg, string describing the syntax error
     *    @return 0 on success.
     */
    int parse_attribute(int     vm_id,
                        string  &attribute,
                        string  &parsed,
                        char ** error_msg);

    /**
     *  Bootstraps the database table(s) associated to the VirtualMachine pool
     */
    void bootstrap()
    {
        VirtualMachine::bootstrap(db);
    };

private:
    /**
     * Mutex to perform just one attribute parse at a time
     */
    static pthread_mutex_t lex_mutex;

    /**
     *  Generate context file to be sourced upon VM booting
     *  @param vm_id, ID of the VM to generate context for
     *  @param attrs, the template CONTEXT attributes (only the first one will
     *  be used)
     */
    void generate_context(int vm_id, vector<Attribute *> attrs);

    /**
     *  Factory method to produce VM objects
     *    @return a pointer to the new VM
     */
    PoolObjectSQL * create()
    {
        return new VirtualMachine;
    };
};

#endif /*VIRTUAL_MACHINE_POOL_H_*/
