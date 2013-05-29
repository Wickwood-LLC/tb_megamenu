<?php

function tb_megamenu_get_all_blocks() {
  global $_blocks_array;
  if (empty($_blocks_array)) {
    $blocks = _block_rehash($GLOBALS['conf']['theme_default']);
    $_blocks_array = array ();
    foreach ($blocks as $block) {
      if ($block ['module'] != 'tb_megamenu') {
        $_blocks_array[$block['bid']] = $block ['info'] . ' (' . $block ['module'] . ':' . $block ['delta'] . ')';
      }
    }
  }
  return $_blocks_array;
}

function tb_megamenu_get_blocks_info() {
  static $_blocks_array = array();
  if (empty($_blocks_array)) {
    $blocks = _block_rehash($GLOBALS['conf']['theme_default']);
    $_blocks_array = array();
    foreach ($blocks as $block) {
      if ($block['module'] != 'tb_megamenu') {
        $_blocks_array[$block['bid']] = $block;
      }
    }
  }
  return $_blocks_array;
}

function tb_megamenu_build_page_trail($page_menu) {
  $path = current_path();
  $trail = array ();
  foreach ($page_menu as $item) {
    if ($item ['link'] ['in_active_trail']) {
      $trail [$item ['link'] ['mlid']] = $item;
    }
    elseif ($item['link']['href'] == '<front>' && drupal_is_front_page()) {
      $trail [$item ['link'] ['mlid']] = $item;
    }
    if ($item ['below']) {
      $trail += tb_megamenu_build_page_trail($item ['below']);
    }
  }
  return $trail;
}

function tb_megamenu_get_megamenus() {
  $query = db_select('menu_custom', 'm');
  $query->leftJoin('tb_megamenus', 't', 't.menu_name = m.menu_name');
  $query->fields('m');
  $menus = $query->execute()->fetchAll();
  return $menus;
}

function tb_megamenu_get_menu($menu_name) {
  $query = db_select('menu_custom', 'm');
  $query->leftJoin('tb_megamenus', 't', 't.menu_name = m.menu_name');
  $query->fields('m');
  $query->addField('t', 'menu_config');
  $query->addField('t', 'block_config');
  $query->condition('m.menu_name', $menu_name);
  return $query->execute()->fetchObject();
}

function tb_megamenu_get_menu_config($menu_name) {
  $menu = tb_megamenu_get_menu($menu_name);
  return $menu && isset($menu->menu_config) ? json_decode($menu->menu_config, true) : array();
}

function tb_megamenu_get_block_config($menu_name) {
  $menu = tb_megamenu_get_menu($menu_name);
  return $menu && isset($menu->block_config) ? json_decode($menu->block_config, true) : array();
}

function tb_megamenu_find_templates($path) {
  $templates = array();
  $regex = '/\.tpl\.php$/';

  $files = drupal_system_listing($regex, $path, 'name', 0);
  foreach ($files as $template => $file) {
    if (($pos = strpos($template, '.')) !== FALSE) {
      $template = substr($template, 0, $pos);
    }
    $templates[$template] = $template;
  }

  return $templates;
}

function tb_megamenu_get_menu_item($menu_name, $mlid) {
  $tree = menu_tree_all_data($menu_name);
  $item = tb_megamenu_find_menu_item($tree, $mlid);
  return $item;
}

function tb_megamenu_find_menu_item($tree, $mlid) {
  foreach($tree as $item) {
    if ($item['link']['mlid'] == $mlid) {
      return $item;
    }
    else {
      $result = tb_megamenu_find_menu_item($item['below'], $mlid);
      if ($result) {
        return $result;
      }
    }
  }
  return NULL;
}

function tb_megamenu_load_block($bid) {
  $blocks = tb_megamenu_get_blocks_info();
  return isset($blocks[$bid]) ? $blocks[$bid] : $blocks[$bid];
}

function tb_megamenu_sync_config_all($items, &$menu_config) {
  foreach($items as $item) {
    $mlid = $item['link']['mlid'];
    $item_config = isset($menu_config[$mlid]) ? $menu_config[$mlid] : array();
    if(!$item['link']['hidden'] && (!empty($item['below']) || !empty($item_config))) {
      tb_megamenu_sync_config($item['below'], $item_config, $mlid);
      $menu_config[$mlid] = $item_config;
      tb_megamenu_sync_config_all($item['below'], $menu_config);
    }
  }
}

function tb_megamenu_sync_config($items, &$item_config, $_mlid) {
  if(empty($item_config)) {
    $item_config = array('rows_content' => array(0 => array(0 => array('col_content' => array(), 'col_config' => array()))), 'submenu_config' => array());
    foreach($items as $item) {
      $mlid = $item['link']['mlid'];
      if(!$item['link']['hidden']) {
        $item_config['rows_content'][0][0]['col_content'][] = array(
          'type' => 'menu_item',
          'mlid' => $mlid,
          'tb_item_config' => array(),
        );
      }
    }
  }
  else {
    $hash = array();
    foreach($item_config['rows_content'] as $i => $row) {
      foreach($row as $j => $col) {
        foreach($col['col_content'] as $k => $tb_item) {
          if ($tb_item['type'] == 'menu_item') {
            $hash[$tb_item['mlid']] = array('row' => $i, 'col' => $j);
            $existed = false;
            foreach ($items as $item) {
              if (!$item['link']['hidden'] && $tb_item['mlid'] == $item['link']['mlid']) {
                $existed = true;
                break;
              }
            }
            if(!$existed) {
              unset($item_config['rows_content'][$i][$j]['col_content'][$k]);
              if(empty($item_config['rows_content'][$i][$j]['col_content'])) {
                unset($item_config['rows_content'][$i][$j]);
              }
              if(empty($item_config['rows_content'][$i])) {
                unset($item_config['rows_content'][$i]);
              }
            }
          }
        }
      }
    }
    $row = -1; $col = -1;
    foreach($items as $item) {
      $mlid = $item['link']['mlid'];
      if(!$item['link']['hidden']) {
        if(isset($hash[$mlid])) {
          $row = $hash[$mlid]['row'];
          $col = $hash[$mlid]['col'];
          continue;
        }
        if($row > -1) {
          $item_config['rows_content'][$row][$col]['col_content'][] = array(
            'mlid' => $mlid,
            'type' => 'menu_item',
            'tb_item_config' => array(),
          );
        }
        else {
          $row = 0;
          $col = 0;
          $item_config['rows_content'][$row][$col]['col_content'][] = array(
            'mlid' => $mlid,
            'type' => 'menu_item',
            'tb_item_config' => array(),
          );
          $item_config['rows_content'][$row][$col]['col_config'] = array();
        }
      }
    }
  }
}

function tb_megamenu_array_to_object($array) {
  $obj = new stdClass;
  foreach($array as $k => $v) {
     if(is_array($v)) {
        $obj->{$k} = tb_megamenu_array_to_object($v);
     } else {
        $obj->{$k} = $v;
     }
  }
  return $obj;
}

function tb_megamenu_edit_item_config(&$item_config) {
  if(!isset($item_config['xicon'])) {
    $item_config['xicon'] = "";
  }
  if(!isset($item_config['class'])) {
    $item_config['class'] = "";
  }
  if(!isset($item_config['caption'])) {
    $item_config['caption'] = "";
  }
  if(!isset($item_config['alignsub'])) {
    $item_config['alignsub'] = "";
  }
  if(!isset($item_config['group'])) {
    $item_config['group'] = 0;
  }
  if(!isset($item_config['hidewcol'])) {
    $item_config['hidewcol'] = 0;
  }
  if(!isset($item_config['hidesub'])) {
    $item_config['hidesub'] = 0;
  }
}

function tb_megamenu_edit_submenu_config(&$submenu_config) {
  if(!isset($submenu_config['width'])) {
    $submenu_config['width'] = "";
  }
  if(!isset($submenu_config['class'])) {
    $submenu_config['class'] = "";
  }
  if(!isset($submenu_config['group'])) {
    $submenu_config['group'] = "";
  }
}

function tb_megamenu_edit_col_config(&$col_config) {
  if(!isset($col_config['width'])) {
    $col_config['width'] = 12;
  }
  if(!isset($col_config['class'])) {
    $col_config['class'] = "";
  }
  if(!isset($col_config['hidewcol'])) {
    $col_config['hidewcol'] = 0;
  }
}

function tb_megamenu_edit_block_config(&$block_config) {
  if(!isset($block_config['animation'])) {
    $block_config['animation'] = "none";
  }
  if(!isset($block_config['style'])) {
    $block_config['style'] = "";
  }
  if(!isset($block_config['auto-arrow'])) {
    $block_config['auto-arrow'] = true;
  }
  if(!isset($block_config['duration'])) {
    $block_config['duration'] = 400;
  }
  if(!isset($block_config['always-show-submenu'])) {
    $block_config['always-show-submenu'] = 1;
  }
  if(!isset($block_config['off-canvas'])) {
    $block_config['off-canvas'] = 0;
  }
  
}

function tb_megamenu_render_block($delta = 0) {
  global $tb_elements_counter;
  $tb_elements_counter = array('column' => 0);
  $block = array('content' => array(
    '#type' => 'markup',
    '#markup' => theme('tb_megamenu', array('menu_name' => $delta, 'section' => 'backend',)),
  ));
  drupal_add_js('Drupal.TBMegaMenu = Drupal.TBMegaMenu || {};', array('type' => 'inline'));
  drupal_add_js('Drupal.TBMegaMenu.TBElementsCounter = ' . json_encode($tb_elements_counter), array('type' => 'inline'));
  drupal_add_js('Drupal.TBMegaMenu.TBElementsCounter = ' . json_encode($tb_elements_counter), array('type' => 'inline'));
  return $block;
}

function tb_megamenu_animation_options($block_config) {
  $animations = array('none' => t('None'), 'fading' => t('Fading'), 'slide' => t('Slide'), 'zoom' => t('Zoom'), 'elastic' => t('Elastic'));
  $options = array();
  foreach($animations as $value => $title) {
    if($value == $block_config['animation']) {
      $options[] = '<option value="' . $value . '" selected="selected">' . $title . '</option>';
    }
    else {
      $options[] = '<option value="' . $value . '">' . $title . '</option>';
    }
  }
  return implode("\n", $options);
}

function tb_megamenu_style_options($block_config) {
  $styles = array(
    '' => t('Default'),
    'black' => t('Black'),
    'blue' => t('Blue'),
	'green' => t('Green'), 
    'orange' => t('Orange'),      
  );
  $options = array();
  foreach($styles as $value => $title) {
    if($value == $block_config['style']) {
      $options[] = '<option value="' . $value . '" selected="selected">' . $title . '</option>';
    }
    else {
      $options[] = '<option value="' . $value . '">' . $title . '</option>';
    }
  }
  return implode("\n", $options);
}

function tb_megamenu_find_hook_templates($cache, $path) {
  $templates = array();
  $regex = '/\.tpl\.php$/';

  // Because drupal_system_listing works the way it does, we check for real
  // templates separately from checking for patterns.
  $files = drupal_system_listing($regex, $path, 'name', 0);
  if(isset($files['tb-megamenu-submenu--default.tpl'])) {
    unset($files['tb-megamenu-submenu--default.tpl']);
  }
  foreach ($files as $template => $file) {
    // Chop off the remaining extensions if there are any. $template already
    // has the rightmost extension removed, but there might still be more,
    // such as with .tpl.php, which still has .tpl in $template at this point.
    if (($pos = strpos($template, '.')) !== FALSE) {
      $template = substr($template, 0, $pos);
    }
    // Transform - in filenames to _ to match function naming scheme
    // for the purposes of searching.
    $hook = strtr($template, '-', '_');
    if (isset($cache[$hook])) {
      $templates[$hook] = array(
        'template' => $template,
        'path' => dirname($file->uri),
        'includes' => isset($cache[$hook]['includes']) ? $cache[$hook]['includes'] : NULL,
      );
    }
    // Ensure that the pattern is maintained from base themes to its sub-themes.
    // Each sub-theme will have their templates scanned so the pattern must be
    // held for subsequent runs.
    if (isset($cache[$hook]['pattern'])) {
      $templates[$hook]['pattern'] = $cache[$hook]['pattern'];
    }
  }

  $patterns = array_keys($files);

  foreach ($cache as $hook => $info) {
    if (!empty($info['pattern'])) {
      // Transform _ in pattern to - to match file naming scheme
      // for the purposes of searching.
      $pattern = strtr($info['pattern'], '_', '-');

      $matches = preg_grep('/^'. $pattern .'/', $patterns);
      if ($matches) {
        foreach ($matches as $match) {
          $file = substr($match, 0, strpos($match, '.'));
          // Put the underscores back in for the hook name and register this pattern.
          $templates[strtr($file, '-', '_')] = array(
            'template' => $file,
            'path' => dirname($files[$match]->uri),
            'variables' => isset($info['variables']) ? $info['variables'] : NULL,
            'render element' => isset($info['render element']) ? $info['render element'] : NULL,
            'base hook' => $hook,
            'includes' => isset($info['includes']) ? $info['includes'] : NULL,
          );
        }
      }
    }
  }
  return $templates;
}

function tb_megamenu_is_a_key($str) {
  $no_keys = array('(', ')', ';', '/', '#tb-megamenu', '@');
  foreach($no_keys as $key) {
    if(strpos($str, $key) !== false) {
      return false;
    }
  }
  $yes_keys = array('.', '-', ',', '{');
  foreach($yes_keys as $key) {
    if(strpos($str, $key) !== false) {
      return true;
    }
  }
  return false;
}

function tb_megamenu_replace_css() {
  $files = array(
    drupal_get_path('module', 'tb_megamenu') . '/css/admin.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/backend.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/base.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/bootstrap.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/bootstrap-responsive.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/default.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/styles/black.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/styles/blue.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/styles/green.css', 
    drupal_get_path('module', 'tb_megamenu') . '/css/styles/orange.css', 
  );
  foreach($files as $css_file) {
    $output = array();
    $f = fopen($css_file, "r");
    $lines = array();
    $ls = array();
    while($line = fgets($f)) {
      $lines[] = $line;
      $line = str_replace("\r\n", "", $line);
      if(tb_megamenu_is_a_key($line)) {
        $line = trim($line);
        if(strpos($line, '.tb-megamenu') !== 0) {
          $ls[] = '.tb-megamenu ' . $line;
        }
        else {
          $ls[] = $line;
        }
      }
      else {
        $ls[] = $line;
      }
    }
    file_put_contents($css_file, implode("\r\n", $ls));
    fclose($f);
  }
}
