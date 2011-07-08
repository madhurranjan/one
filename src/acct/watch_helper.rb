module WatchHelper
    require 'sequel'
    require 'yaml'

    ONE_LOCATION=ENV["ONE_LOCATION"]

    if !ONE_LOCATION
        ACCTD_CONF="/etc/one/acctd.conf"
    else
        ACCTD_CONF=ONE_LOCATION+"/etc/acctd.conf"
    end

    CONF = YAML.load_file(ACCTD_CONF)

    DB = Sequel.connect(CONF[:DB])
    VM_DELTA = {
        :net_rx => {
            :type => Integer,
            :path => 'NET_RX'
        },
        :net_tx => {
            :type => Integer,
            :path => 'NET_TX'
        }
    }

    VM_SAMPLE = {
        :cpu => {
            :type => Integer,
            :path => 'CPU'
        },
        :memory => {
            :type => Integer,
            :path => 'MEMORY'
        },
        :net_tx => {
            :type => Integer,
            :path => 'NET_TX'
        },
        :net_rx => {
            :type => Integer,
            :path => 'NET_RX'
        }
    }

    HOST_SAMPLE = {
        :disk_usage => {
            :type => Integer,
            :path => 'DISK_USAGE'
        },
        :mem_usage => {
            :type => Integer,
            :path => 'MEM_USAGE'
        },
        :cpu_usage => {
            :type => Integer,
            :path => 'CPU_USAGE'
        },
        :max_disk => {
            :type => Integer,
            :path => 'MAX_DISK'
        },
        :max_mem => {
            :type => Integer,
            :path => 'MAX_MEM'
        },
        :max_cpu => {
            :type => Integer,
            :path => 'MAX_CPU'
        },
        :free_disk => {
            :type => Integer,
            :path => 'FREE_DISK'
        },
        :free_mem => {
            :type => Integer,
            :path => 'FREE_MEM'
        },
        :free_cpu => {
            :type => Integer,
            :path => 'FREE_CPU'
        },
        :used_disk => {
            :type => Integer,
            :path => 'USED_DISK'
        },
        :used_mem => {
            :type => Integer,
            :path => 'USED_MEM'
        },
        :used_cpu => {
            :type => Integer,
            :path => 'USED_CPU'
        },
        :rvms => {
            :type => Integer,
            :path => 'RUNNING_VMS'
        }
    }

    def self.get_config(*params)
        conf = CONF
        while param = params.shift
            conf = conf[param]
            break if conf.nil?
        end
        conf
    end

    def self.bootstrap
        DB.create_table? :vms do
            Integer :id, :primary_key=>true
            String  :name
            Integer :uid
            Integer :gid
            Integer :mem
            Integer :cpu
            Integer :vcpu
            Integer :stime
            Integer :etime
        end

        DB.create_table? :hosts do
            Integer :id, :primary_key=>true
            String  :name
            String  :im_mad
            String  :vm_mad
            String  :tm_mad
        end

        DB.create_table? :vm_samples do
            foreign_key :vm_id, :vms, :key=>:id
            Integer :state
            Integer :lcm_state
            Integer :last_poll
            Integer :timestamp

            VM_SAMPLE.each { |key,value|
                column key, value[:type]
            }

            primary_key [:vm_id, :timestamp]
        end

        DB.create_table? :host_samples do
            foreign_key :host_id, :hosts, :key=>:id
            Integer :last_poll
            Integer :timestamp
            Integer :state

            HOST_SAMPLE.each { |key,value|
                column key, value[:type]
            }

            primary_key [:host_id, :timestamp]
        end

        DB.create_table? :registers do
            foreign_key :vm_id, :vms, :key=>:id
            Integer     :hid
            String      :hostname
            Integer     :seq
            Integer     :pstime
            Integer     :petime
            Integer     :rstime
            Integer     :retime
            Integer     :estime
            Integer     :eetime
            Integer     :reason

            primary_key [:vm_id, :seq]
        end

        DB.create_table? :vm_deltas do
            foreign_key :vm_id, :vms, :key=>:id
            Integer     :timestamp
            Integer     :ptimestamp

            VM_DELTA.each { |key,value|
                column key, value[:type]
            }

            primary_key [:vm_id, :timestamp]
        end
    end

    self.bootstrap

    class VmSample < Sequel::Model
        unrestrict_primary_key

        many_to_one :vm

        def self.active
            self.filter(:state=>3)
        end

        def self.error
            self.filter(:state=>7)
        end
    end

    class HostSample < Sequel::Model
        unrestrict_primary_key

        many_to_one :host

        def self.active
            self.filter('state < 3')
        end

        def self.error
            self.filter(:state=>3)
        end
    end

    class Register < Sequel::Model
        unrestrict_primary_key

        many_to_one :vm

        def update_from_history(history)
            self.seq      = history['SEQ']
            self.hostname = history['HOSTNAME']
            self.hid      = history['HID']
            self.pstime   = history['PSTIME']
            self.petime   = history['PETIME']
            self.rstime   = history['RSTIME']
            self.retime   = history['RETIME']
            self.estime   = history['ESTIME']
            self.eetime   = history['EETIME']
            self.reason   = history['REASON']

            self.save
        end
    end

    class VmDelta < Sequel::Model
        unrestrict_primary_key

        many_to_one :vm
    end

    class Vm < Sequel::Model
        unrestrict_primary_key

        # Accounting
        one_to_many :registers, :order=>:seq
        one_to_many :deltas, :order=>:timestamp, :class=>VmDelta

        # Monitoring
        one_to_many :samples, :order=>:timestamp, :class=>VmSample

        @@samples_cache = []
        @@deltas_cache  = []

        @@vm_window_size = WatchHelper::get_config(
            :VM_MONITORING,
            :WINDOW_SIZE
        )

        def self.info(vm)
            Vm.find_or_create(:id=>vm['ID']) { |v|
                v.name  = vm['NAME']
                v.uid   = vm['UID'].to_i
                v.gid   = vm['GID'].to_i
                v.mem   = vm['MEMORY'].to_i
                v.cpu   = vm['CPU'].to_i
                v.vcpu  = vm['VCPU'].to_i
                v.stime = vm['STIME'].to_i
                v.etime = vm['ETIME'].to_i
            }
        end

        def add_register_from_resource(history)
            self.add_register(
                :seq      => history['SEQ'],
                :hostname => history['HOSTNAME'],
                :hid      => history['HID'],
                :pstime   => history['PSTIME'],
                :petime   => history['PETIME'],
                :rstime   => history['RSTIME'],
                :retime   => history['RETIME'],
                :estime   => history['ESTIME'],
                :eetime   => history['EETIME'],
                :reason   => history['REASON']
            )
        end

        def add_sample_from_resource(vm, timestamp)
            hash = {
                :vm_id      => vm['ID'],
                :timestamp  => timestamp,
                :last_poll  => vm['LAST_POLL'],
                :state      => vm['STATE'],
                :lcm_state  => vm['LCM_STATE']
            }

            VM_SAMPLE.each { |key,value|
                hash[key] = vm[value[:path]]
            }

            @@samples_cache << hash
        end

        def add_delta_from_resource(vm, timestamp)
            hash = Hash.new

            hash[:vm_id]     = vm['ID']
            hash[:timestamp] = timestamp

            if last_delta = self.deltas.first
                hash[:ptimestamp] = last_delta.send(:timestamp)

                VM_DELTA.each { |key,value|
                    old_value = last_delta.send("#{key}".to_sym)
                    new_value = vm[value[:path]].to_i

                    if old_value > new_value
                        hash[key] = new_value
                    else
                        hash[key] = new_value - old_value
                    end
                }
            else
                hash[:ptimestamp] = 0

                VM_DELTA.each { |key,value|
                    hash[key] = vm[value[:path]]
                }
            end

            @@deltas_cache << hash
        end

        def self.flush
            VmDelta.multi_insert(@@deltas_cache)
            VmSample.multi_insert(@@samples_cache)

            Vm.each { |vm|
                if vm.samples.count > @@vm_window_size -1
                    vm.samples.last.delete
                end
            }

            @@samples_cache = []
            @@deltas_cache = []
        end
    end

    class Host < Sequel::Model
        unrestrict_primary_key

        # Monitoring
        one_to_many :samples, :order=>:timestamp, :class=>HostSample

        @@samples_cache = []

        @@host_window_size = WatchHelper::get_config(
            :HOST_MONITORING,
            :WINDOW_SIZE
        )


        def self.info(host)
            Host.find_or_create(:id=>host['ID']) { |h|
                h.name   = host['NAME']
                h.im_mad = host['IM_MAD']
                h.vm_mad = host['VM_MAD']
                h.tm_mad = host['TM_MAD']
            }
        end

        def add_sample_from_resource(host, timestamp)
            hash = {
                :host_id    => host['ID'],
                :timestamp  => timestamp,
                :last_poll  => host['LAST_MON_TIME'],
                :state      => host['STATE'],
            }

            host_share = host['HOST_SHARE']
            HOST_SAMPLE.each { |key,value|
                hash[key] = host_share[value[:path]]
            }

            @@samples_cache << hash
        end

        def self.flush
            HostSample.multi_insert(@@samples_cache)

            Host.all.each { |host|
                if host.samples.count > @@host_window_size
                    host.samples.first.delete
                end
            }

            @@samples_cache = []
        end
    end
end
