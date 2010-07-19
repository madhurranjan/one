# -------------------------------------------------------------------------- #
# Copyright 2002-2010, OpenNebula Project Leads (OpenNebula.org)             #
#                                                                            #
# Licensed under the Apache License, Version 2.0 (the "License"); you may    #
# not use this file except in compliance with the License. You may obtain    #
# a copy of the License at                                                   #
#                                                                            #
# http://www.apache.org/licenses/LICENSE-2.0                                 #
#                                                                            #
# Unless required by applicable law or agreed to in writing, software        #
# distributed under the License is distributed on an "AS IS" BASIS,          #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
# See the License for the specific language governing permissions and        #
# limitations under the License.                                             #
#--------------------------------------------------------------------------- #

# Common cloud libs
require 'rubygems'
require 'sinatra'
require 'CloudServer'

# OCA
require 'OpenNebula'
include OpenNebula

# OCCI libs
require 'VirtualMachineOCCI'
require 'VirtualMachinePoolOCCI'
require 'VirtualNetworkOCCI'
require 'VirtualNetworkPoolOCCI'
require 'ImageOCCI'
require 'ImagePoolOCCI'

require 'pp'


##############################################################################
# The OCCI Server provides an OCCI implementation based on the
# OpenNebula Engine
##############################################################################
class OCCIServer < CloudServer

    # Server initializer
    # config_file:: _String_ path of the config file
    # template:: _String_ path to the location of the templates
    def initialize(config_file,template)
        super(config_file)

        @config.add_configuration_value("TEMPLATE_LOCATION",template)

        if @config[:ssl_server]
            @base_url=@config[:ssl_server]
        else
            @base_url="http://#{@config[:server]}:#{@config[:port]}"
        end

        print_configuration
    end

    # Retrieve a client with the user credentials
    # requestenv:: _Hash_ Hash containing the environment of the request
    # [return] _Client_ client with the user credentials
    def get_client(requestenv)
        auth =  Rack::Auth::Basic::Request.new(requestenv)

        return one_client_user(auth.credentials[0], auth.credentials[1])
    end

    # Prepare the OCCI XML Response
    # resource:: _Pool_ or _PoolElement_ that represents a OCCI resource
    # [return] _String_,_Integer_ Resource Representation or error, status code
    def to_occi_xml(resource)
        xml_response = resource.to_occi(@base_url)
        return xml_response, 500 if OpenNebula.is_error?(xml_response)

        return xml_response, 201
    end

    ############################################################################
    ############################################################################
    #                      POOL RESOURCE METHODS
    ############################################################################
    ############################################################################

    # Gets the pool representation of COMPUTES
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Pool Representation or error, status code
    def get_computes(request)
        # --- Get client with user credentials ---
        client = get_client(request.env)

        # --- Get User's VMs ---
        user_flag = -1
        vmpool = VirtualMachinePoolOCCI.new(client, user_flag)

        # --- Prepare XML Response ---
        rc = vmpool.info
        return rc, 404 if OpenNebula.is_error?(rc)

        return to_occi_xml(vmpool)
    end


    # Gets the pool representation of NETWORKS
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Network pool representation or error,
    # =>                          status code
    def get_networks(request)
        # --- Get client with user credentials ---
        client = get_client(request.env)

        # --- Get User's VNETs ---
        user_flag = -1
        network_pool = VirtualNetworkPoolOCCI.new(client, user_flag)

        rc = network_pool.info
        return rc, 404 if OpenNebula.is_error?(rc)

        # --- Prepare XML Response ---
        return to_occi_xml(network_pool)
    end

    # Gets the pool representation of STORAGES
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Image pool representation or error,
    #                             status code
    def get_storages(request)
        # --- Get client with user credentials ---
        client = get_client(request.env)

        # --- Get User's Images ---
        user_flag = -1
        image_pool = ImagePoolOCCI.new(client, user_flag)

        result = image_pool.info
        return result, 404 if OpenNebula.is_error?(result)

        # --- Prepare XML Response ---
        return to_occi_xml(image_pool)
    end

    ############################################################################
    ############################################################################
    #                      ENTITY RESOURCE METHODS
    ############################################################################
    ############################################################################

    ############################################################################
    # COMPUTE Methods
    ############################################################################

    # Post a new compute to the COMPUTE pool
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ COMPUTE Representation or error, status code
    def post_compute(request)

        # --- Create the new Instance ---
        vm = VirtualMachineOCCI.new(
                    VirtualMachine.build_xml,
                    get_client(request.env),
                    request.body.read,
                    @instance_types,
                    @config[:template_location])

        # --- Generate the template and Allocate the new Instance ---
        template = vm.to_one_template
        return template, 500 if OpenNebula.is_error?(template)

        rc = vm.allocate(template)
        return rc, 500 if OpenNebula.is_error?(rc)

        # --- Prepare XML Response ---
        vm.info
        return to_occi_xml(vm)
    end

    # Get the representation of a COMPUTE resource
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ COMPUTE representation or error,
    #                             status code
    def get_compute(request, params)

        # --- Get the VM ---
        vm = VirtualMachineOCCI.new(
                VirtualMachine.build_xml(params[:id]),
                get_client(request.env))

        result = vm.info
        return result, 404 if OpenNebula::is_error?(result)

        # --- Prepare XML Response ---
        return to_occi_xml(vm)
    end


    # Deletes a COMPUTE resource
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Delete confirmation msg or error,
    #                             status code
    def delete_compute(request, params)

        # --- Get the VM ---
        vm = VirtualMachineOCCI.new(
                VirtualMachine.build_xml(params[:id]),
                get_client(request.env))

        # --- Finalize the VM ---
        result = vm.finalize
        return result, 500 if OpenNebula::is_error?(result)

        return "", 204
    end

    # Updates a COMPUTE resource
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Update confirmation msg or error,
    #                             status code
    def put_compute(request, params)
        vm_info = XMLUtilsElement.initialize_xml(request.body.read, 'COMPUTE')

        # --- Get the VM and Action on it ---
        if vm_info['STATE'] != nil
            vm = VirtualMachineOCCI.new(
                    VirtualMachine.build_xml(params[:id]),
                    get_client(request.env))

            rc = vm.mk_action(vm_info['STATE'])

            return rc, 400 if OpenNebula.is_error?(rc)
        else
            error_msg = "State not defined in the OCCI XML"
            error = OpenNebula::Error.new(error_msg)
            return error, 400
        end

        # --- Prepare XML Response ---
        vm.info
        return to_occi_xml(vm)
    end

    ############################################################################
    # NETWORK Methods
    ############################################################################

    # Post a new network to the NETWORK pool
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Network Representation or error, status code
    def post_network(request)

        # --- Create the new Instance ---
        network = VirtualNetworkOCCI.new(
                    VirtualNetwork.build_xml,
                    get_client(request.env),
                    request.body,
                    @config[:bridge])

        # --- Generate the template and Allocate the new Instance ---
        template = network.to_one_template
        return template, 500 if OpenNebula.is_error?(template)

        rc = network.allocate(template)
        return rc, 500 if OpenNebula.is_error?(rc)

        # --- Prepare XML Response ---
        network.info
        return to_occi_xml(network)
    end

    # Retrieves a NETWORK resource
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ NETWORK occi representation or error,
    #                             status code
    def get_network(request, params)
        # --- Get the VM ---
        network = VirtualNetworkOCCI.new(
                VirtualNetwork.build_xml(params[:id]),
                get_client(request.env))

        result = network.info
        return result, 404 if OpenNebula::is_error?(result)

        # --- Prepare XML Response ---
        return to_occi_xml(network)
    end

    # Deletes a NETWORK resource
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Delete confirmation msg or error,
    #                             status code
    def delete_network(request, params)

        vn = VirtualNetworkOCCI.new(
                VirtualNetwork.build_xml(params[:id]),
                get_client(request.env))

        # --- Delete the VNET ---
        result = vn.delete
        return result, 500 if OpenNebula::is_error?(result)

        return "", 204
    end

    ############################################################################
    # STORAGE Methods
    ############################################################################

    # Post a new image to the STORAGE pool
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Image representation or error, status code
    def post_storage(request)

        # --- Check OCCI XML from POST ---
        if request.params['occixml'] == nil
            error_msg = "OCCI XML representation of Image" +
                        " not present in the request"
            error = OpenNebula::Error.new(error_msg)
            return error, 400
        end

        # --- Create and Add the new Image ---
        image = ImageOCCI.new(Image.build_xml,
                              get_client(request.env),
                              request.params['occixml'])

        rc = add_image(image, request.params['file'])
        return rc, 500 if OpenNebula.is_error?(rc)

        # --- Enable the new Image ---
        rc = image.enable
        return rc, 500 if OpenNebula.is_error?(rc)

        # --- Prepare XML Response ---
        return to_occi_xml(image)
    end

    # Get a STORAGE resource
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ STORAGE occi representation or error,
    #                             status code
    def get_storage(request, params)

        # --- Get the Image ---
        image = ImageOCCI.new(Image.build_xml(params[:id]),
                              get_client(request.env))

        result = image.info
        return result, 404 if OpenNebula::is_error?(result)

        # --- Prepare XML Response ---
        return to_occi_xml(image)
    end

    # Deletes a STORAGE resource (Not yet implemented)
    # request:: _Hash_ hash containing the data of the request
    # [return] _String_,_Integer_ Delete confirmation msg or error,
    #                             status code
    def delete_storage(request, params)

        image = ImageOCCI.new(Image.build_xml(params[:id]),
                              get_client(request.env))

        # --- Delete the Image ---
        result = image.delete
        return result, 500 if OpenNebula::is_error?(result)

        return "", 204
    end
end
