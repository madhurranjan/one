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

require 'openssl'
require 'base64'
require 'fileutils'

require 'x509_auth'

# Server authentication class. This authmethod can be used by opennebula services
# to let access authenticated users by other means. It is based on x509 server 
# certificates
class ServerAuth < X509Auth
    ###########################################################################
    #Constants with paths to relevant files and defaults
    ###########################################################################
    if !ENV["ONE_LOCATION"]
        ETC_LOCATION      = "/etc/one"
    else
        ETC_LOCATION      = ONE_LOCATION + "/etc"
    end

    SERVER_AUTH_CONF_PATH = ETC_LOCATION + "/auth/server_auth.conf"

    DEFAULT_CERTS_PATH = {
        :one_cert => ETC_LOCATION + "/auth/cert.pem",
        :one_key  => ETC_LOCATION + "/auth/key.pem",
        :ca_dir   => ETC_LOCATION + "/auth/certificates",
    }

    ###########################################################################

    def initialize()
        @options = DEFAULT_CERTS_PATH

        if File.readable?(SERVER_AUTH_CONF_PATH)
            config = File.read(SERVER_AUTH_CONF_PATH)
             
            @options.merge!(YAML::load(config))
        end
        
        begin
            certs = [ File.read(@options[:one_cert]) ]
            key   =   File.read(@options[:one_key])

            super(:certs_pem => certs, 
                  :key_pem   => key,
                  :ca_dir    => @options[:ca_dir])
        rescue
            raise
        end  
    end

    # Generates a login token in the form:
    # user_name:server:user_name:user_pass:time_expires
    #   - user_name:user_pass:time_expires is encrypted with the server certificate
    def login_token(user, user_pass, expire)

        expires = Time.now.to_i+expire
        
        token_txt = "#{user}:#{user_pass}:#{expires}"

        token     = encrypt(token_txt)
	    token64   = Base64::encode64(token).strip.delete("\n")

        login_out = "#{user}:server:#{token64}"
        
        login_out
    end

    ###########################################################################
    # Server side
    ###########################################################################
    # auth method for auth_mad
    def authenticate(user, pass, signed_text)
        begin            
	        # Decryption demonstrates that the user posessed the private key.
            _user, user_pass, expires = decrypt(signed_text).split(':')

            return "User name missmatch" if user != _user

            return "login token expired" if Time.now.to_i >= expires.to_i

            # Check an explicitly-specified DN such as for a host-signed login 
            if !pass.split('|').include?(user_pass)
	            return "User password missmatch"
            end

	        validate

            return true
        rescue => e
            return e.message
        end
    end
end
