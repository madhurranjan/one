/* ------------------------------------------------------------------------ */
/* Copyright 2002-2010, OpenNebula Project Leads (OpenNebula.org)           */
/*                                                                          */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may  */
/* not use this file except in compliance with the License. You may obtain  */
/* a copy of the License at                                                 */
/*                                                                          */
/* http://www.apache.org/licenses/LICENSE-2.0                               */
/*                                                                          */
/* Unless required by applicable law or agreed to in writing, software      */
/* distributed under the License is distributed on an "AS IS" BASIS,        */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/* See the License for the specific language governing permissions and      */
/* limitations under the License.                                           */
/* ------------------------------------------------------------------------ */

#include <limits.h>
#include <string.h>

#include <iostream>
#include <sstream>

#include "Host.h"
#include "NebulaLog.h"

/* ************************************************************************ */
/* Host :: Constructor/Destructor                                           */
/* ************************************************************************ */

Host::Host(
    int     id,
    string _hostname,
    string _im_mad_name,
    string _vmm_mad_name,
    string _tm_mad_name):
        PoolObjectSQL(id,table),
        hostname(_hostname),
        state(INIT),
        im_mad_name(_im_mad_name),
        vmm_mad_name(_vmm_mad_name),
        tm_mad_name(_tm_mad_name),
        last_monitored(0),
        cluster(ClusterPool::DEFAULT_CLUSTER_NAME),
        host_template()
        {}

Host::~Host(){}

/* ************************************************************************ */
/* Host :: Database Access Functions                                        */
/* ************************************************************************ */

const char * Host::table = "host_pool";

const char * Host::db_names = "oid, name, body, state, last_mon_time";

const char * Host::db_bootstrap = "CREATE TABLE IF NOT EXISTS host_pool ("
    "oid INTEGER PRIMARY KEY, name VARCHAR(256), body TEXT, state INTEGER, "
    "last_mon_time INTEGER, UNIQUE(name))";

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int Host::insert(SqlDB *db, string& error_str)
{
    int rc;

    rc = insert_replace(db, false);

    if ( rc != 0 )
    {
        error_str = "Error inserting Host in DB.";
    }

    return rc;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int Host::update(SqlDB *db)
{
    int    rc;

    rc = insert_replace(db, true);

    return rc;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int Host::insert_replace(SqlDB *db, bool replace)
{
    ostringstream   oss;

    int    rc;
    string xml_body;

    char * sql_hostname;
    char * sql_xml;

   // Update the Host

    sql_hostname = db->escape_str(hostname.c_str());

    if ( sql_hostname == 0 )
    {
        goto error_hostname;
    }

    sql_xml = db->escape_str(to_xml(xml_body).c_str());

    if ( sql_xml == 0 )
    {
        goto error_body;
    }

    if(replace)
    {
        oss << "REPLACE";
    }
    else
    {
        oss << "INSERT";
    }

    // Construct the SQL statement to Insert or Replace

    oss <<" INTO "<<table <<" ("<< db_names <<") VALUES ("
        <<          oid                 << ","
        << "'" <<   sql_hostname        << "',"
        << "'" <<   sql_xml             << "',"
        <<          state               << ","
        <<          last_monitored      << ")";

    rc = db->exec(oss);

    db->free_str(sql_hostname);
    db->free_str(sql_xml);

    return rc;

error_body:
    db->free_str(sql_hostname);
error_hostname:
    return -1;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int Host::update_info(string &parse_str)
{
    char *  error_msg;
    int     rc;

    rc = host_template.parse(parse_str, &error_msg);

    if ( rc != 0 )
    {
        NebulaLog::log("ONE", Log::ERROR, error_msg);

        free(error_msg);
        return -1;
    }

    get_template_attribute("TOTALCPU",host_share.max_cpu);
    get_template_attribute("TOTALMEMORY",host_share.max_mem);

    get_template_attribute("FREECPU",host_share.free_cpu);
    get_template_attribute("FREEMEMORY",host_share.free_mem);

    get_template_attribute("USEDCPU",host_share.used_cpu);
    get_template_attribute("USEDMEMORY",host_share.used_mem);

    return 0;
}

/* ************************************************************************ */
/* Host :: Misc                                                             */
/* ************************************************************************ */

ostream& operator<<(ostream& os, Host& host)
{
    string host_str;

    os << host.to_xml(host_str);

    return os;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

string& Host::to_xml(string& xml) const
{
    string template_xml;
    string share_xml;
    ostringstream   oss;

    oss <<
    "<HOST>"
       "<ID>"            << oid       	   << "</ID>"            <<
       "<NAME>"          << hostname 	   << "</NAME>"          <<
       "<STATE>"         << state          << "</STATE>"         <<
       "<IM_MAD>"        << im_mad_name    << "</IM_MAD>"        <<
       "<VM_MAD>"        << vmm_mad_name   << "</VM_MAD>"        <<
       "<TM_MAD>"        << tm_mad_name    << "</TM_MAD>"        <<
       "<LAST_MON_TIME>" << last_monitored << "</LAST_MON_TIME>" <<
       "<CLUSTER>"       << cluster        << "</CLUSTER>"       <<
       host_share.to_xml(share_xml)  <<
       host_template.to_xml(template_xml) <<
    "</HOST>";

    xml = oss.str();

    return xml;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int Host::from_xml(const string& xml)
{
    vector<xmlNodePtr> content;

    // Initialize the internal XML object
    update_from_str(xml);

    oid         = atoi(((*this)["/HOST/ID"] )[0].c_str() );
    hostname    = ((*this)["/HOST/NAME"])[0];
    state       = static_cast<HostState>(  atoi(((*this)["/HOST/STATE"])[0].c_str())  );

//  TODO: create an ObjectXML method to allow this syntax:
//    im_mad_name = xpath("/HOST/IM_MAD", "im_default");

    im_mad_name  = ((*this)["/HOST/IM_MAD"])[0];
    vmm_mad_name = ((*this)["/HOST/VM_MAD"])[0];
    tm_mad_name  = ((*this)["/HOST/TM_MAD"])[0];


    last_monitored = static_cast<time_t>(  atoi(((*this)["/HOST/LAST_MON_TIME"] )[0].c_str() )  );

    cluster = ((*this)["/HOST/CLUSTER"])[0];

    ObjectXML::get_nodes("/HOST/HOST_SHARE", content);
    host_share.from_xml_node( content[0] );

    content.clear();
    ObjectXML::get_nodes("/HOST/TEMPLATE", content);
    host_template.from_xml_node( content[0] );

    // TODO: check for errors (missing mandatory elements)
    return 0;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

string& Host::to_str(string& str) const
{
    string template_str;
    string share_str;

    ostringstream   os;

    os <<
        "ID      =  "  << oid            << endl <<
        "NAME = "      << hostname       << endl <<
        "STATE    = "  << state          << endl <<
        "IM MAD   = "  << im_mad_name    << endl <<
        "VMM MAD  = "  << vmm_mad_name   << endl <<
        "TM MAD   = "  << tm_mad_name    << endl <<
        "LAST_MON = "  << last_monitored << endl <<
        "CLUSTER  = "  << cluster        << endl <<
        "ATTRIBUTES"   << endl << host_template.to_str(template_str) << endl <<
        "HOST SHARES"  << endl << host_share.to_str(share_str) <<endl;

    str = os.str();

    return str;
}
