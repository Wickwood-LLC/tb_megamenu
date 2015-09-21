<?php

function tb_megamenu_request() {
  $action = isset($_POST['action']) ? $_POST['action'] : NULL;
  switch($action) {
    case 'load':
      $block = tb_megamenu_render_block($_POST['menu_name']);
      print($block['content']['#markup']);
      break;

    case 'save':
      global $language;
      $menu_config = isset($_POST['menu_config']) ? $_POST['menu_config'] : NULL;
      $block_config = isset($_POST['block_config']) ? $_POST['block_config'] : NULL;
      $menu_name = isset($_POST['menu_name']) ? $_POST['menu_name'] : NULL;
      if($menu_config && $menu_name) {
        // Invoke modules which implement hook_tb_megamenu_save_config().
        foreach (module_implements('tb_megamenu_save_config') as $module) {
          $function = $module . '_tb_megamenu_save_config';
          if (function_exists($function)) {
            $function($menu_name, $language->language, $_POST);
          }
        }
        $primary_keys = db_select('tb_megamenus', 't')
                        ->fields('t', array('menu_name', 'language'))
                        ->condition('menu_name', $menu_name)
                        ->condition('language', $language->language)
                        ->execute()
                        ->fetchAssoc();
        $record = array(
          'menu_name' => $menu_name,
          'block_config' => $block_config,
          'menu_config' => $menu_config,
          'language' => $language->language
        );
        print drupal_write_record('tb_megamenus', $record, $primary_keys ? array(
          'menu_name',
          'language'
        ) : NULL);
      }
      break;

    case 'load_block':
      $block_key = isset($_POST['block_key']) ? $_POST['block_key'] : NULL;
      $id = isset($_POST['id']) ? $_POST['id'] : NULL;
      $showblocktitle = isset($_POST['showblocktitle']) ? $_POST['showblocktitle'] : NULL;
      if($block_key) {
        $content = json_encode(array(
          'content' => theme('tb_megamenu_block', array(
            'block_key' => $block_key,
            'section' => 'backend',
            'showblocktitle' => $showblocktitle)
          ), 'id' => $id)
        );
        print($content);
      }
      break;

    default:
      break;
  }
  exit;
}
