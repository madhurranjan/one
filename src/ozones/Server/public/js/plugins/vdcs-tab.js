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

var vdcs_tab_content =
'<form id="form_vdcs" action="javascript:alert(\'js errors?!\')">\
  <div class="action_blocks">\
  </div>\
<table id="datatable_vdcs" class="display">\
  <thead>\
    <tr>\
      <th class="check"><input type="checkbox" class="check_all" value="">All</input></th>\
      <th>ID</th>\
      <th>Name</th>\
      <th>Zone ID</th>\
    </tr>\
  </thead>\
  <tbody id="tbodyvdcs">\
  </tbody>\
</table>\
</form>';

var create_vdc_tmpl =
'<form id="create_vdc_form" action="">\
  <fieldset>\
     <div>\
        <label for="name">Name:</label>\
        <input type="text" name="name" id="name" /><br />\
        <label for="vdcadminname">Admin name:</label>\
        <input type="text" name="vdcadminname" id="vdcadminname" /><br />\
        <label for="vdcadminpass">Admin pass:</label>\
        <input type="password" name="vdcadminpass" id="vdcadminpass" />\
        <label for="zone">Zone:</label>\
        <select id="zoneid" name="zone">\
        </select>\
        <label style="margin-left:195px;font-size:0.8em;color:#bbbbbb">Available / Selected</label>\
        <label>Hosts:</label>\
        <div id="vdc_hosts_lists" class="dd_lists">\
          <ul id="vdc_available_hosts_list" class="dd_list dd_left"></ul>\
          <ul id="vdc_selected_hosts_list" class="dd_list dd_right"></ul>\
     </div>\
   </fieldset>\
   <fieldset>\
     <div class="form_buttons">\
        <button class="button" id="create_vdc_submit" value="Zone.create">Create</button>\
        <button class="button" type="reset" value="reset">Reset</button>\
     </div>\
   </fieldset>\
</form>';

var dataTable_vdcs;

function vdcSelectedNodes() {
    return getSelectedNodes(dataTable_vdcs);
};

var vdc_actions = {
    "VDC.create" : {
        type: "create",
        call: oZones.VDC.create,
        callback: addVDCElement,
        error: onError,
        notify: true
    },

    "VDC.create_dialog" : {
        type: "custom",
        call: openCreateVDCDialog
    },

    "VDC.list" : {
        type: "list",
        call: oZones.VDC.list,
        callback: updateVDCsView,
        error: onError,
    },

    "VDC.refresh" : {
        type: "custom",
        call: function() {
            waitingNodes(dataTable_vdcs);
            Sunstone.runAction("Zone.list");
        },
        callback: Empty,
        error: onError
    },

    "VDC.autorefresh" : {
        type: "custom",
        call: function(){
            oZones.VDC.list({timeout: true, success: updateVDCsView, error: onError});
        },
    },

    "VDC.delete" : {
        type: "multiple",
        call: oZones.VDC.delete,
        callback: deleteVDCElement,
        elements: vdcSelectedNodes,
        error: onError,
        notify: true
    },
    "VDC.showinfo" : {
        type: "single",
        call: oZones.VDC.show,
        callback: updateVDCInfo,
        error: onError
    },
    "VDC.zone_hosts" : {
        type: "single",
        call: oZones.Zone.host,
        callback: fillHostList,
        error: onError
    }
};

var vdc_buttons = {
    "VDC.refresh" : {
        type: "image",
        text: "Refresh list",
        img: "images/Refresh-icon.png"
    },
    "VDC.create_dialog" : {
        type: "action",
        text: "+ New",
        alwaysActive:true
    },
    "VDC.delete" : {
        type: "action",
        text: "Delete",
        type : "confirm",
        tip: "Careful! This will delete the selected VDCs and associated resources"
    }
};

var vdcs_tab = {
    title: "VDCs",
    content: vdcs_tab_content,
    buttons: vdc_buttons
}

var vdc_info_panel = {
    "vdc_info_tab" : {
        title: "VDC Information",
        content: ""
    }
};

Sunstone.addActions(vdc_actions);
Sunstone.addMainTab("vdcs_tab",vdcs_tab);
Sunstone.addInfoPanel("vdc_info_panel",vdc_info_panel);

function vdcElementArray(vdc_json){
    var vdc = vdc_json.VDC;

    return [
        '<input type="checkbox" id="vdc_'+vdc.id+'" name="selected_items" value="'+vdc.id+'"/>',
        vdc.id,
        vdc.name,
        vdc.zones_id
    ];
}

function vdcInfoListener() {
   $("#tbodyvdcs tr").live("click", function(e){
        if ($(e.target).is('input')) {return true;}
        popDialogLoading();
        var aData = dataTable_vdcs.fnGetData(this);
        var id = $(aData[0]).val();
        Sunstone.runAction("VDC.showinfo",id);
        return false;
    });
}

function deleteVDCElement(req){
    deleteElement(dataTable_vdcs,'#vdc_'+req.request.data);
}

function addVDCElement(req,vdc_json){
    var element = vdcElementArray(vdc_json);
    addElement(element,dataTable_vdcs);
}

function updateVDCsView(req,vdc_list){
    var vdc_list_array = [];

    $.each(vdc_list,function(){
        vdc_list_array.push(vdcElementArray(this));
    });

    updateView(vdc_list_array,dataTable_vdcs);
    updateZonesDashboard("vdcs",vdc_list);
}

function updateVDCInfo(req,vdc_json){
    var vdc = vdc_json.VDC;
    var info_tab = {
        title: "VDC Information",
        content :
           '<table id="info_vdc_table" class="info_table">\
            <thead>\
               <tr><th colspan="2">Virtual Data Center - '+vdc.name+'</th></tr>\
            </thead>\
            <tbody>\
            <tr>\
                <td class="key_td">ID</td>\
                <td class="value_td">'+vdc.id+'</td>\
            </tr>\
            <tr>\
                <td class="key_td">Name</td>\
                <td class="value_td">'+vdc.name+'</td>\
            </tr>\
            <tr>\
                <td class="key_td">Zone ID</td>\
                <td class="value_td">'+vdc.zones_id+'</td>\
            </tr>\
            <tr>\
                <td class="key_td">Hosts</td>\
                <td class="value_td">'+(vdc.hosts? vdc.hosts : "none")+'</td>\
            </tr>\
            </tbody>\
         </table>'
    };

    Sunstone.updateInfoPanelTab("vdc_info_panel","vdc_info_tab",info_tab);
    Sunstone.popUpInfoPanel("vdc_info_panel");
}

function fillHostList(req, host_list_json){
    list = "";
    $.each(host_list_json,function(){
        list+='<li host_id="'+this.HOST.ID+'">'+this.HOST.NAME+'</li>';
    });
    $('div#create_vdc_dialog #vdc_available_hosts_list').html(list);
}

function setupCreateVDCDialog(){
    $('div#dialogs').append('<div title="Create VDC" id="create_vdc_dialog"></div>');
    var dialog = $('div#create_vdc_dialog');
    dialog.html(create_vdc_tmpl);
    dialog.dialog({
        autoOpen: false,
        modal: true,
        width: 500
    });

    $('button',dialog).button();
    $('#vdc_available_hosts_list',dialog).sortable({
        connectWith : '#vdc_selected_hosts_list',
        containment: dialog
    });
    $('#vdc_selected_hosts_list',dialog).sortable({
        connectWith : '#vdc_available_hosts_list',
        containment: dialog
    });


    //load zone hosts
    $('select#zoneid').change(function(){
        var id=$(this).val();
        var av_hosts=
            $('div#create_vdc_dialog #vdc_available_hosts_list');
        var sel_hosts=
            $('div#create_vdc_dialog #vdc_selected_hosts_list');
        av_hosts.html('<li>'+spinner+'</li>');
        sel_hosts.empty();
        Sunstone.runAction("VDC.zone_hosts",id);
    });

    $('#create_vdc_form', dialog).submit(function(){
        var name = $('#name',$(this)).val();
        var vdcadminname = $('#vdcadminname',$(this)).val();
        var vdcadminpass = $('#vdcadminpass',$(this)).val();
        var zoneid = $('select#zoneid',$(this)).val();
        if (!name.length || !vdcadminname.length
            || !vdcadminpass.length || !zoneid.length){
            notifyError("Name, administrator credentials or zones are missing");
            return false;
        }
        var hosts="";
        $('#vdc_selected_hosts_list li',$(this)).each(function(){
            hosts+=$(this).text()+',';
        });
        if (hosts.length){
            hosts= hosts.slice(0,-1);
        };

        var vdc_json = {
            "vdc" : {
                "name" : name,
                "zoneid" : zoneid,
                "vdcadminname" : vdcadminname,
                "vdcadminpass" : vdcadminpass,
            }
        };
        if (hosts.length){
            vdc_json["vdc"]["hosts"]=hosts;
        };

        Sunstone.runAction("VDC.create",vdc_json);
        dialog.dialog('close');
        return false;
    });
}

function openCreateVDCDialog(){
    var dialog = $('div#create_vdc_dialog')
    $('select#zoneid',dialog).html(zones_select);
    $('#vdc_available_hosts_list',dialog).html("<li>No hosts available</li>");
    $('#vdc_selected_hosts_list',dialog).empty();
    dialog.dialog('open');
}

function setVDCAutorefresh() {
    setInterval(function(){
        var checked = $('input:checked',dataTable_zones.fnGetNodes());
        var  filter = $("#datatable_vdcs_filter input").attr("value");
        if (!checked.length && !filter.length){
            Sunstone.runAction("VDC.autorefresh");
        }
    },INTERVAL+someTime());
}

$(document).ready(function(){
    dataTable_vdcs = $("#datatable_vdcs").dataTable({
        "bJQueryUI": true,
        "bSortClasses": false,
        "bAutoWidth":false,
        "sPaginationType": "full_numbers",
        "aoColumnDefs": [
            { "bSortable": false, "aTargets": ["check"] },
            { "sWidth": "60px", "aTargets": [0] },
            { "sWidth": "35px", "aTargets": [1,3] }
        ]
    });

    dataTable_vdcs.fnClearTable();
    addElement([spinner,'','',''],dataTable_vdcs);
    Sunstone.runAction("VDC.list");

    setupCreateVDCDialog();
    setVDCAutorefresh();
    initCheckAllBoxes(dataTable_vdcs);
    tableCheckboxesListener(dataTable_vdcs);
    vdcInfoListener();
});