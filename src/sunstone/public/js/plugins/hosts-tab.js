/* -------------------------------------------------------------------------- */
/* Copyright 2002-2011, OpenNebula Project Leads (OpenNebula.org)             */
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

/*Host tab plugin*/

var host_tab_content = 
'<form id="form_hosts" action="javascript:alert(\'js errors?!\')">\
  <div class="action_blocks">\
  </div>\
<table id="datatable_hosts" class="display">\
  <thead>\
    <tr>\
      <th class="check"><input type="checkbox" class="check_all" value="">All</input></th>\
      <th>ID</th>\
      <th>Name</th>\
      <th>Cluster</th>\
      <th>Running VMs</th>\
      <th>CPU Use</th>\
      <th>Memory use</th>\
      <th>Status</th>\
    </tr>\
  </thead>\
  <tbody id="tbodyhosts">\
  </tbody>\
</table>\
</form>';

var hosts_select="";
var host_list_json = {};


//Setup actions
var host_actions = {
            "Host.create" : {
                type: "create",
                call : OpenNebula.Host.create,
                callback = addHostElement,
                error : onError,
                notify:true,
                condition: true
            },
            
            "Host.enable" = {
                type: "multiple",
                call : OpenNebula.Host.enable,
                callback = host_update_callback,
                error : onError,
                notify:true,
                condition:true
            },
            
            "Host.disable" = {
                type: "multiple",
                call : OpenNebula.Host.disable,
                callback = host_update_callback,
                error : onError,
                notify:true,
                condition:true
            },
            
            "Host.delete" = {
                type: "multiple",
                call : OpenNebula.Host.create,
                callback = deleteHostElement,
                error : onError,
                notify:true,
                condition:true
            },
            
            "Host.list" = {
                type: "custom",
                call : function() {
                    OpenNebula.Host.list({success: updateHostsView, error: onError});
                    OpenNebula.Cluster.list({success: updateClustersView, error: onError});
                    }
                callback: null,
                error: onError,
                notify:true,
                condition:true 
            },
            
            "Cluster.create" = {
                type: "create",
                call : OpenNebula.Cluster.create,
                callback = addClusterElement
                error : onError,
                notify:true,
                condition : true
            },
            
            "Cluster.delete" = {
                type: "multiple",
                call : OpenNebula.Host.create,
                callback = addHostElement,
                error : onError,
                notify:true,
                condition:true
            },
            
            "Cluster.addhost" = {
                type: "confirm_with_select",
                select: cluster_select,
                tip: "Select the cluster in which you would like to place the hosts",
                call : OpenNebula.Cluster.addhost,
                callback = updateHostElement,
                error : onError,
                notify:true,
                condition : true
            },
            
            "Cluster.removehost" = {
                type: "multiple",
                call : OpenNebula.Cluster.removehost,
                callback = deleteHostElement,
                error : onError,
                notify:true,
                condition:true
            }
        };


var host_buttons = [
        {
            type: "create",
            text: "+ New host",
            action: "Host.create",
            condition : true
        },
        {
            type: "action",
            text: "Enable",
            action: "Host.enable",
            condition : true
        },
        {
            type: "action",
            text: "Disable",
            action: "Host.disable",
            condition : true
        },
        {
            type: "create",
            text: "+ New Cluster",
            action: "Cluster.create",
            condition : true
        },
        {
            type: "action",
            text: "Delete cluster",
            action: "Cluster.delete",
            condition : true
        }
        {
            type: "select",
            action: [{ text: "Add host to cluster", 
                        value: "Cluster.addhost",
                        condition: true},
                     {  text: "Remove host from cluster",
                        value: "Cluster.removehost",
                        condition: true}],
            condition : true
        },
        {
            type: "action",
            text: "Delete host",
            value: "Host.delete",
            condition : true
        }]};
            
for (action in host_actions){
    Sunstone.addAction(action,host_actions[action]);
}

Sunstone.addMainTab(hosts_tab_content,'hosts_tab');

$.each(host_buttons,function(){
    Sunstone.addButton(this,'#hosts_tab');
}


//Setup tab


//Plugin functions
function host_update_callback(req){
    OpenNebula.Host.show({data:{id:req.request.data[0]},success: updateHostElement,error: onError});
}

function hostElementArray(host_json){
	host = host_json.HOST;
	acpu = parseInt(host.HOST_SHARE.MAX_CPU);
		if (!acpu) {acpu=100};
	acpu = acpu - parseInt(host.HOST_SHARE.CPU_USAGE);

    total_mem = parseInt(host.HOST_SHARE.MAX_MEM);
    free_mem = parseInt(host.HOST_SHARE.FREE_MEM);

    if (total_mem == 0) {
        ratio_mem = 0;
    } else {
        ratio_mem = Math.round(((total_mem - free_mem) / total_mem) * 100);
    }


    total_cpu = parseInt(host.HOST_SHARE.MAX_CPU);
    used_cpu = Math.max(total_cpu - parseInt(host.HOST_SHARE.USED_CPU),acpu);

    if (total_cpu == 0) {
        ratio_cpu = 0;
    } else {
        ratio_cpu = Math.round(((total_cpu - used_cpu) / total_cpu) * 100);
    }

     pb_mem =
'<div style="height:10px" class="ratiobar ui-progressbar ui-widget ui-widget-content ui-corner-all" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="'+ratio_mem+'">\
    <div class="ui-progressbar-value ui-widget-header ui-corner-left ui-corner-right" style="width: '+ratio_mem+'%;"/>\
    <span style="position:relative;left:45px;top:-4px;font-size:0.6em">'+ratio_mem+'%</span>\
    </div>\
</div>';

    pb_cpu =
'<div style="height:10px" class="ratiobar ui-progressbar ui-widget ui-widget-content ui-corner-all" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="'+ratio_cpu+'">\
    <div class="ui-progressbar-value ui-widget-header ui-corner-left ui-corner-right" style="width: '+ratio_cpu+'%;"/>\
    <span style="position:relative;left:45px;top:-4px;font-size:0.6em">'+ratio_cpu+'%</span>\
    </div>\
</div>';


    return [ '<input type="checkbox" id="host_'+host.ID+'" name="selected_items" value="'+host.ID+'"/>',
			host.ID,
			host.NAME,
			host.CLUSTER,
			host.HOST_SHARE.RUNNING_VMS, //rvm
            pb_cpu,
			pb_mem,
			OpenNebula.Helper.resource_state("host",host.STATE) ];


	//~ return [ '<input type="checkbox" id="host_'+host.ID+'" name="selected_items" value="'+host.ID+'"/>',
			//~ host.ID,
			//~ host.NAME,
			//~ host.CLUSTER,
			//~ host.HOST_SHARE.RUNNING_VMS, //rvm
			//~ host.HOST_SHARE.MAX_CPU, //tcpu
			//~ parseInt(host.HOST_SHARE.MAX_CPU) - parseInt(host.HOST_SHARE.USED_CPU), //fcpu
			//~ acpu,
			//~ humanize_size(host.HOST_SHARE.MAX_MEM),
			//~ humanize_size(host.HOST_SHARE.FREE_MEM),
			//~ OpenNebula.Helper.resource_state("host",host.STATE) ];
}


function hostInfoListener(){
	$('#tbodyhosts tr').live("click",function(e){

		//do nothing if we are clicking a checkbox!
		if ($(e.target).is('input')) {return true;}

        popDialogLoading();
		aData = dataTable_hosts.fnGetData(this);
		id = $(aData[0]).val();
		OpenNebula.Host.show({data:{id:id},success: updateHostInfo,error: onError});
		return false;
	});
}

function updateHostSelect(host_list){

	//update select helper
	hosts_select="";
	hosts_select += "<option value=\"\">Select a Host</option>";
	$.each(host_list, function(){
		hosts_select += "<option value=\""+this.HOST.ID+"\">"+this.HOST.NAME+"</option>";
	});

	//update static selectors
	$('#vm_host').html(hosts_select);
}


function updateHostElement(request, host_json){
	id = host_json.HOST.ID;
	element = hostElementArray(host_json);
	updateSingleElement(element,dataTable_hosts,'#host_'+id);
}

function deleteHostElement(req){
	deleteElement(dataTable_hosts,'#host_'+req.request.data);
}

function addHostElement(request,host_json){
    id = host_json.HOST.ID;
	element = hostElementArray(host_json);
	addElement(element,dataTable_hosts);
}

function updateHostsView (request,host_list){
	host_list_json = host_list;
	host_list_array = []

	$.each(host_list,function(){
	//Grab table data from the host_list
		host_list_array.push(hostElementArray(this));
	});

	updateView(host_list_array,dataTable_hosts);
	updateHostSelect(host_list);
	updateDashboard("hosts",host_list_json);
}

function updateHostInfo(request,host){
	host_info = host.HOST
	rendered_info =
'<div id="host_informations">\
	<ul>\
		<li><a href="#info_host">Host information</a></li>\
		<li><a href="#host_template">Host template</a></li>\
	</ul>\
	<div id="info_host">\
		<table id="info_host_table" class="info_table">\
			<thead>\
				<tr><th colspan="2">Host information - '+host_info.NAME+'</th></tr>\
			</thead>\
			<tr>\
				<td class="key_td">ID</td>\
				<td class="value_td">'+host_info.ID+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">State</td>\
				<td class="value_td">'+OpenNebula.Helper.resource_state("host",host_info.STATE)+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">Cluster</td>\
				<td class="value_td">'+host_info.CLUSTER+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">IM MAD</td>\
				<td class="value_td">'+host_info.IM_MAD+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">VM MAD</td>\
				<td class="value_td">'+host_info.VM_MAD+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">TM MAD</td>\
				<td class="value_td">'+host_info.TM_MAD+'</td>\
			</tr>\
		</table>\
		<table id="host_shares_table" class="info_table">\
			<thead>\
				<tr><th colspan="2">Host shares</th></tr>\
			</thead>\
			<tr>\
				<td class="key_td">Max Mem</td>\
				<td class="value_td">'+humanize_size(host_info.HOST_SHARE.MAX_MEM)+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">Used Mem (real)</td>\
				<td class="value_td">'+humanize_size(host_info.HOST_SHARE.USED_MEM)+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">Used Mem (allocated)</td>\
				<td class="value_td">'+humanize_size(host_info.HOST_SHARE.MAX_USAGE)+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">Used CPU (real)</td>\
				<td class="value_td">'+host_info.HOST_SHARE.USED_CPU+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">Used CPU(allocated)</td>\
				<td class="value_td">'+host_info.HOST_SHARE.CPU_USAGE+'</td>\
			</tr>\
			<tr>\
				<td class="key_td">Running VMs</td>\
				<td class="value_td">'+host_info.HOST_SHARE.RUNNING_VMS+'</td>\
			</tr>\
		</table>\
	</div>\
	<div id="host_template">\
		<table id="host_template_table" class="info_table">\
		<thead><tr><th colspan="2">Host template</th></tr></thead>'+
		prettyPrintJSON(host_info.TEMPLATE)+
		'</table>\
	</div>\
</div>';
    popDialog(rendered_info);
    $('#host_informations').tabs();

}

//Document ready
$(document).ready(){
    
    //prepare host datatable
    var dataTable_hosts = $("#datatable_hosts").dataTable({
      "bJQueryUI": true,
      "bSortClasses": false,
      "bAutoWidth":false,
      "sPaginationType": "full_numbers",
      "aoColumnDefs": [
                        { "bSortable": false, "aTargets": ["check"] },
                        { "sWidth": "60px", "aTargets": [0,4] },
                        { "sWidth": "35px", "aTargets": [1] },
                        { "sWidth": "120px", "aTargets": [5,6] }
                       ]
    });
    
    //preload it
    dataTable_hosts.fnClearTable();
    addElement([
        spinner,
        '','','','','','',''],dataTable_hosts);
	OpenNebula.Host.list({success: updateHostsView,error: onError});
    
    //set refresh interval
    setInterval(function(){
		nodes = $('input:checked',dataTable_hosts.fnGetNodes());
        filter = $("#datatable_hosts_filter input").attr("value");
		if (!nodes.length && !filter.length){
			OpenNebula.Host.list({timeout: true, success: updateHostsView,error: onError});
		}
	},interval);
    
    initCheckAllBoxes(dataTable_hosts);
    
    //.action button listener
    $('#hosts_tab .action_button').click(function(){
        Sunstone.runActionOnDatatableNodes($(this).val(),dataTable_hosts);
    }
    
}
