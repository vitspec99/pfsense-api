<?php

namespace RESTAPI\Caches;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Cache;
use function RESTAPI\Dispatchers\get_pkg_info;

/**
 * Defines a Cache for available pfSense package information obtained from the pfSense package repos.
 */
class AvailablePackageCache extends Cache {
    public int $timeout = 120;
    public string $schedule = '12 * * * *'; # Run at irregular interval to avoid conflicts with repo jobs
    /**
     * Retrieves the available package information to cache from external repos
     */
    public function get_data_to_cache(): array {
        return \get_pkg_info();
    }
}
