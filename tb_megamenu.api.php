<?php

/**
 * Register extension for TB Mega Menu.
 */
function hook_tb_megamenu_extension_info() {
  return array(
    'machine_name' => array(
      'title' => t('Name of extension'),
      'type' => 'theme'
    )
  );
}

/**
 * Set default value to configuration.
 */
function hook_tb_megamenu_block_config_info() {
  return array(
    'config-item' => 'value'
  );
}