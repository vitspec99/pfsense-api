<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model for HAProxy Backend Servers.
 */
class HAProxyBackendServer extends Model {
    public StringField $name;
    public StringField $status;
    public StringField $address;
    public PortField $port;
    public IntegerField $weight;
    public BooleanField $ssl;
    public BooleanField $sslserververify;
    public IntegerField $serverid;
    public StringField $advanced;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxyBackend';
        $this->config_path = 'ha_servers/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            minimum_length: 2,
            validators: [new RegexValidator(pattern: '/[^a-zA-Z0-9\.\-_]/', invert: true)],
            help_text: 'The unique name for this backend server.',
        );
        $this->status = new StringField(
            default: 'active',
            choices: ['active', 'backup', 'disabled', 'inactive'],
            help_text: 'The eligibility status for this backend server.',
        );
        $this->address = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            help_text: 'The hostname or IP address of this backend server. Hostname values are only resolved at ' .
                'service startup.',
        );
        $this->port = new PortField(
            required: true,
            allow_alias: false,
            allow_range: false,
            help_text: 'The port to forward to for this backend server.',
        );
        $this->weight = new IntegerField(
            default: 1,
            minimum: 0,
            maximum: 256,
            help_text: 'The weight of this backend server when load balancing.',
        );
        $this->ssl = new BooleanField(
            default: false,
            indicates_true: 'yes',
            help_text: 'Enables or disables using SSL/TLS when forwarding to this backend server.',
        );
        $this->sslserververify = new BooleanField(
            default: false,
            indicates_true: 'yes',
            help_text: 'Enables or disables verifying the SSL/TLS certificate when forwarding to this backend server.',
        );
        $this->serverid = new IntegerField(
            default: rand(100, 1000), # TODO: Ensure this value is unique from Backends and Backend servers
            editable: false,
            read_only: true,
            help_text: 'The unique ID for this backend server. This value is set by the system for internal use and ' .
                'cannot be changed.',
        );
        $this->advanced = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'Allows adding custom HAProxy server settings to the server.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Ensures haproxy is marked as dirty before applying.
     */
    public function pre_apply(): void {
        touch('/var/run/haproxy.conf.dirty');
    }

    /**
     * Applies changes to the HAProxy configuration.
     */
    public function apply(): void {
        (new HAProxyApplyDispatcher(async: $this->async))->spawn_process();
    }
}
