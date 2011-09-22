module EC2CloudAuth
    def auth(env, params={})
        username = params['AWSAccessKeyId']
        one_pass = get_password(username)
        return "Invalid credentials" unless one_pass

        signature = case params['SignatureVersion']
            when "1" then signature_v1(params.clone,one_pass)
            when "2" then signature_v2(params.clone,one_pass,env,true,false)
        end

        if params['Signature'] != signature
            if params['SignatureVersion']=="2"
                signature = signature_v2(params.clone,one_pass,env,false,false)
                if params['Signature'] != signature
                    return "Invalid Credentials"
                end
            else
                return "Invalid Credentials"
            end
        end

        @token = "#{username}:#{one_pass}"
        @client = Client.new(@token, @conf[:one_xmlrpc], false)
        return nil
    end

    private

    # Calculates signature version 1
    def signature_v1(params, secret_key, digest='sha1')
        params.delete('Signature')
        req_desc = params.sort {|x,y| x[0].downcase <=> y[0].downcase}.to_s

        digest_generator = OpenSSL::Digest::Digest.new(digest)
        digest = OpenSSL::HMAC.digest(digest_generator, secret_key, req_desc)
        b64sig = Base64.b64encode(digest)
        return b64sig.strip
    end

    # Calculates signature version 2
    def signature_v2(params, secret_key, env, include_port=true, urlencode=true)
        params.delete('Signature')
        params.delete('file')

        server_host = params.delete(:econe_host)
        server_port = params.delete(:econe_port)
        if include_port
            server_str = "#{server_host}:#{server_port}"
        else
            server_str = server_host
        end

        canonical_str = AWS.canonical_string(
                                params,
                                server_str,
                                env['REQUEST_METHOD'])

        # Use the correct signature strength
        sha_strength = case params['SignatureMethod']
            when "HmacSHA1"   then 'sha1'
            when "HmacSHA256" then 'sha256'
            else 'sha1'
        end

        digest  = OpenSSL::Digest::Digest.new(sha_strength)
        hmac    = OpenSSL::HMAC.digest(digest, secret_key, canonical_str)
        b64hmac = Base64.encode64(hmac).gsub("\n","")

        if urlencode
            return CGI::escape(b64hmac)
        else
            return b64hmac
        end
    end
end
