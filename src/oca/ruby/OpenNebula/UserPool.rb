# -------------------------------------------------------------------------- #
# Copyright 2002-2011, OpenNebula Project Leads (OpenNebula.org)             #
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

require 'OpenNebula/Pool'

module OpenNebula
    class UserPool < Pool
        # ---------------------------------------------------------------------
        # Constants and Class attribute accessors
        # ---------------------------------------------------------------------

        USER_POOL_METHODS = {
            :info => "userpool.info"
        }

        # ---------------------------------------------------------------------
        # Class constructor & Pool Methods
        # ---------------------------------------------------------------------
        
        # +client+ a Client object that represents a XML-RPC connection
        def initialize(client)
            super('USER_POOL','USER',client)
        end

        # Factory method to create User objects
        def factory(element_xml)
            OpenNebula::User.new(element_xml,@client)
        end

        # ---------------------------------------------------------------------
        # XML-RPC Methods for the User Object
        # ---------------------------------------------------------------------

        # Retrieves all the Users in the pool.
        def info()
            super(USER_POOL_METHODS[:info])
        end
    end
end
