<?php

/**
 * Register extension for TB Mega Menu.
 */
function hook_tb_megamenu_extension_info() {
  $example_info = array(
    'machine_name' => array(
      'title' => t('Name of extension'),
      'type' => 'theme'
    )
  );
  return $example_info;
}