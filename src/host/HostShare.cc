/* ------------------------------------------------------------------------*/
/* Copyright 2002-2010, OpenNebula Project Leads (OpenNebula.org)          */
/*                                                                         */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may */
/* not use this file except in compliance with the License. You may obtain */
/* a copy of the License at                                                */
/*                                                                         */
/* http://www.apache.org/licenses/LICENSE-2.0                              */
/*                                                                         */
/* Unless required by applicable law or agreed to in writing, software     */
/* distributed under the License is distributed on an "AS IS" BASIS,       */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*/
/* See the License for the specific language governing permissions and     */
/* limitations under the License.                                          */
/* ------------------------------------------------------------------------*/

#include <limits.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <algorithm>

#include "HostShare.h"

/* ************************************************************************ */
/* HostShare :: Constructor/Destructor                                      */
/* ************************************************************************ */

HostShare::HostShare(
        int     _max_disk,
        int     _max_mem,
        int     _max_cpu):
        ObjectXML(),
        disk_usage(0),
        mem_usage(0),
        cpu_usage(0),
        max_disk(_max_disk),
        max_mem(_max_mem),
        max_cpu(_max_cpu),
        free_disk(0),
        free_mem(0),
        free_cpu(0),
        used_disk(0),
        used_mem(0),
        used_cpu(0),
        running_vms(0){};

ostream& operator<<(ostream& os, HostShare& hs)
{
    string str;

    os << hs.to_xml(str);

    return os;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

string& HostShare::to_xml(string& xml) const
{
    string template_xml;
    ostringstream   oss;

    oss << "<HOST_SHARE>"
          << "<DISK_USAGE>" << disk_usage << "</DISK_USAGE>"
          << "<MEM_USAGE>"  << mem_usage  << "</MEM_USAGE>"
          << "<CPU_USAGE>"  << cpu_usage  << "</CPU_USAGE>"
          << "<MAX_DISK>"   << max_disk   << "</MAX_DISK>"
          << "<MAX_MEM>"    << max_mem    << "</MAX_MEM>"
          << "<MAX_CPU>"    << max_cpu    << "</MAX_CPU>"
          << "<FREE_DISK>"  << free_disk  << "</FREE_DISK>"
          << "<FREE_MEM>"   << free_mem   << "</FREE_MEM>"
          << "<FREE_CPU>"   << free_cpu   << "</FREE_CPU>"
          << "<USED_DISK>"  << used_disk  << "</USED_DISK>"
          << "<USED_MEM>"   << used_mem   << "</USED_MEM>"
          << "<USED_CPU>"   << used_cpu   << "</USED_CPU>"
          << "<RUNNING_VMS>"<<running_vms <<"</RUNNING_VMS>"
        << "</HOST_SHARE>";

    xml = oss.str();

    return xml;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int HostShare::from_xml_node(const xmlNodePtr node)
{
    // Initialize the internal XML object
    ObjectXML::update_from_node(node);

    disk_usage = atoi(((*this)["/HOST_SHARE/DISK_USAGE"] )[0].c_str() );
    mem_usage  = atoi(((*this)["/HOST_SHARE/MEM_USAGE"] )[0].c_str() );
    cpu_usage  = atoi(((*this)["/HOST_SHARE/CPU_USAGE"] )[0].c_str() );

    max_disk = atoi(((*this)["/HOST_SHARE/MAX_DISK"] )[0].c_str() );
    max_mem  = atoi(((*this)["/HOST_SHARE/MAX_MEM"] )[0].c_str() );
    max_cpu  = atoi(((*this)["/HOST_SHARE/MAX_CPU"] )[0].c_str() );

    free_disk = atoi(((*this)["/HOST_SHARE/FREE_DISK"] )[0].c_str() );
    free_mem  = atoi(((*this)["/HOST_SHARE/FREE_MEM"] )[0].c_str() );
    free_cpu  = atoi(((*this)["/HOST_SHARE/FREE_CPU"] )[0].c_str() );

    used_disk = atoi(((*this)["/HOST_SHARE/USED_DISK"] )[0].c_str() );
    used_mem  = atoi(((*this)["/HOST_SHARE/USED_MEM"] )[0].c_str() );
    used_cpu  = atoi(((*this)["/HOST_SHARE/USED_CPU"] )[0].c_str() );

    running_vms = atoi(((*this)["/HOST_SHARE/RUNNING_VMS"] )[0].c_str() );

    return 0;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

string& HostShare::to_str(string& str) const
{
    string template_xml;
    ostringstream   oss;

    oss<< "\tCPU_USAGE    = " << cpu_usage << endl
       << "\tMEMORY_USAGE = " << mem_usage << endl
       << "\tDISK_USAGE   = " << disk_usage<< endl
       << "\tMAX_CPU      = " << max_cpu << endl
       << "\tMAX_MEMORY   = " << max_mem << endl
       << "\tMAX_DISK     = " << max_disk<< endl
       << "\tFREE_CPU     = " << free_cpu << endl
       << "\tFREE_MEMORY  = " << free_mem << endl
       << "\tFREE_DISK    = " << free_disk<< endl
       << "\tUSED_CPU     = " << used_cpu << endl
       << "\tUSED_MEMORY  = " << used_mem << endl
       << "\tUSED_DISK    = " << used_disk<< endl
       << "\tRUNNING_VMS  = " << running_vms<< endl;

    str = oss.str();

    return str;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
