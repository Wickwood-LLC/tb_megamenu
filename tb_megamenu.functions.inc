<?php

function tb_megamenu_check_library($module, $name) {
  $files = array();
  if ($library = libraries_detect($name)) {
    if (!empty($library['files']['css'])) {
      $files['css'] = libraries_get_path($name) . '/' . key($library['files']['css']);
    }
    if (!empty($library['js'])) {
      $files['js'] = libraries_get_path($name) . '/' . key($library['files']['js']);
    }
  }
  elseif ($library = drupal_get_library($module, $name)) {
    if (!empty($library['css'])) {
      $files['css'] = key($library['css']);
    }
    if (!empty($library['js'])) {
      $files['js'] = key($library['js']);
    }
  }
  return !empty($files) ? $files : FALSE;
}

function tb_megamenu_add_css($file) {
  $path_info = pathinfo($file);
  if ($path_info['extension'] == 'less') {
    $uri = "public://tb_megamenu";
    if (!is_dir($uri)) {
      mkdir($uri, 0777);
    }
    $css_uri = "public://tb_megamenu/css";
    if (!is_dir($css_uri)) {
      mkdir($css_uri, 0777);
    }
    if (!class_exists('lessc')) {
      include_once drupal_get_path('module', 'tb_megamenu') . '/includes/lessphp/lessc.inc.php';
    }
    $less = new lessc();
    $output = $less->compileFile($file);
    file_put_contents($css_uri . "/" . $path_info['filename'] . ".css", $output);
    drupal_add_css($css_uri . "/" . $path_info['filename'] . ".css", array(
      'group' => CSS_DEFAULT,
      'basename' => 'tb-megamenu-' . $path_info['filename'] . ".css",
    ));
  }
  else {
    drupal_add_css($file, array(
      'group' => CSS_DEFAULT,
      'basename' => 'tb-megamenu-' . $path_info['basename'],
    ));
  }
}

function tb_megamenu_get_blocks_info() {
  static $_blocks_array = array();
  if (empty($_blocks_array)) {
    $blocks = db_select('block', 'b')->fields('b')->execute()->fetchAll();
    $_blocks_array = array();
    foreach ($blocks as $block) {
      if ($block->module != 'tb_megamenu') {
        $_blocks_array[$block->module . '--' . $block->delta] = $block;
      }
    }
  }
  return $_blocks_array;
}

function tb_megamenu_get_blocks_options() {
  static $_blocks_array = array();
  if (empty($_blocks_array)) {
    $blocks = _block_rehash($GLOBALS['conf']['theme_default']);
    $_blocks_array = array();
    foreach ($blocks as $block) {
      if ($block['module'] != 'tb_megamenu') {
        $_blocks_array[$block ['module'] . '--' . $block ['delta']] = empty($block['info']) ? $block['title'] : $block['info'];
      }
    }
    asort($_blocks_array);
  }
  return $_blocks_array;
}

function tb_megamenu_get_block_title($block_key) {
  $blocks_options = tb_megamenu_get_blocks_options();
  return isset($blocks_options[$block_key]) ? $blocks_options[$block_key] : NULL;
}

function tb_megamenu_build_page_trail($page_menu) {
  $trail = array();
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
  global $language;
  $query = db_select('menu_custom', 'm');
  $query->leftJoin('tb_megamenus', 't', 't.menu_name = m.menu_name');
  $query->fields('m');
  $query->addField('t', 'menu_config');
  $query->addField('t', 'block_config');
  $query->condition('m.menu_name', $menu_name);
  $query->condition('t.language', $language->language);
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

function tb_megamenu_get_tree($menu_name) {
  static $trees = array();
  if (!isset($trees[$menu_name])) {
    global $menu_admin;
    $sql = "
      SELECT m.load_functions, m.to_arg_functions, m.access_callback, m.access_arguments, m.page_callback, m.page_arguments, m.delivery_callback, m.title, m.title_callback, m.title_arguments, m.type, m.description, ml.*
      FROM {menu_links} ml LEFT JOIN {menu_router} m ON m.path = ml.router_path
      WHERE ml.menu_name = :menu
      ORDER BY p1 ASC, p2 ASC, p3 ASC, p4 ASC, p5 ASC, p6 ASC, p7 ASC, p8 ASC, p9 ASC";
    $result = db_query($sql, array(':menu' => $menu_name), array('fetch' => PDO::FETCH_ASSOC));
    $links = array();
    foreach ($result as $item) {
      $item['href'] = $item['link_path'];
      $links[] = $item;
    }
    $items = menu_tree_data($links);
    $node_links = array();
    menu_tree_collect_node_links($items, $node_links);
    $menu_admin = TRUE;
    menu_tree_check_access($items, $node_links);
    $menu_admin = FALSE;
    $trees[$menu_name] = $items;
  }
  return $trees[$menu_name];
}

function tb_megamenu_get_menu_item($menu_name, $mlid) {
  $tree = tb_megamenu_get_tree($menu_name);
  if (function_exists('i18n_menu_localize_tree')) {
    $tree = i18n_menu_localize_tree($tree);
  }
  $item = tb_megamenu_find_menu_item($tree, $mlid);
  return $item;
}

function tb_megamenu_find_menu_item($tree, $mlid) {
  foreach ($tree as $item) {
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

function tb_megamenu_load_block($block_key) {
  $blocks = tb_megamenu_get_blocks_info();
  return isset($blocks[$block_key]) ? $blocks[$block_key] : NULL;
}

function tb_megamenu_sync_config_all($items, &$menu_config, $section) {
  foreach ($items as $item) {
    $mlid = $item['link']['mlid'];
    $item_config = isset($menu_config[$mlid]) ? $menu_config[$mlid] : array();
    if (!$item['link']['hidden'] && (!empty($item['below']) || !empty($item_config))) {
      tb_megamenu_sync_config($item['below'], $item_config, $mlid, $section);
      $menu_config[$mlid] = $item_config;
      tb_megamenu_sync_config_all($item['below'], $menu_config, $section);
    }
  }
}

function tb_megamenu_sync_order_menus(&$menu_config) {
  foreach ($menu_config as $mlid => $config) {
    foreach ($config['rows_content'] as $rows_id => $row) {
      $item_sorted = array();
      // Get weight from items.
      foreach ($row as $col) {
        foreach ($col['col_content'] as $menu_item) {
          if ($menu_item['type'] == 'menu_item') {
            $item_sorted[$menu_item['weight']] = $menu_item;
          }
        }
      }
      ksort($item_sorted); // Sort menu by weight.
      foreach ($row as $rid => $col) {
        foreach ($col['col_content'] as $menu_item_id => $menu_item) {
          if ($menu_item['type'] == 'menu_item') {
            $menu_config[$mlid]['rows_content'][$rows_id][$rid]['col_content'][$menu_item_id] = array_shift($item_sorted);
          }
        }
      }
    }
  }
}

function tb_megamenu_block_content_exists($block_key, $section) {
  $block = tb_megamenu_load_block($block_key);
  if ($block) {
    $module = $block->module;
    $delta = $block->delta;
    $content = module_invoke($module, 'block_view', $delta);
    if ($content || $section == 'backend') {
      return TRUE;
    }
  }
  return FALSE;
}

function tb_megamenu_sync_config($items, &$item_config, $_mlid, $section) {
  if (empty($item_config['rows_content'])) {
    $item_config['rows_content'] = array(0 => array(0 => array('col_content' => array(), 'col_config' => array())));
    foreach ($items as $item) {
      $mlid = $item['link']['mlid'];
      if (!$item['link']['hidden']) {
        $item_config['rows_content'][0][0]['col_content'][] = array(
          'type' => 'menu_item',
          'mlid' => $mlid,
          'tb_item_config' => array(),
          'weight' => $item['link']['weight'],
        );
      }
    }
    if (empty($item_config['rows_content'][0][0]['col_content'])) {
      unset($item_config['rows_content'][0]);
    }
  }
  else {
    $hash = array();
    foreach ($item_config['rows_content'] as $i => $row) {
      foreach ($row as $j => $col) {
        foreach ($col['col_content'] as $k => $tb_item) {
          if ($tb_item['type'] == 'menu_item') {
            $hash[$tb_item['mlid']] = array('row' => $i, 'col' => $j);
            $existed = false;
            foreach ($items as $item) {
              if (!$item['link']['hidden'] && $tb_item['mlid'] == $item['link']['mlid']) {
                $item_config['rows_content'][$i][$j]['col_content'][$k]['weight'] = $item['link']['weight'];
                $existed = true;
                break;
              }
            }
            if (!$existed) {
              unset($item_config['rows_content'][$i][$j]['col_content'][$k]);
              if (empty($item_config['rows_content'][$i][$j]['col_content'])) {
                unset($item_config['rows_content'][$i][$j]);
              }
              if (empty($item_config['rows_content'][$i])) {
                unset($item_config['rows_content'][$i]);
              }
            }
          }
          else {
            if (!tb_megamenu_block_content_exists($tb_item['block_key'], $section)) {
              unset($item_config['rows_content'][$i][$j]['col_content'][$k]);
              if (empty($item_config['rows_content'][$i][$j]['col_content'])) {
                unset($item_config['rows_content'][$i][$j]);
              }
              if (empty($item_config['rows_content'][$i])) {
                unset($item_config['rows_content'][$i]);
              }
            }
          }
        }
      }
    }
    $row = -1;
    $col = -1;
    foreach ($items as $item) {
      $mlid = $item['link']['mlid'];
      if (!$item['link']['hidden']) {
        if (isset($hash[$mlid])) {
          $row = $hash[$mlid]['row'];
          $col = $hash[$mlid]['col'];
          continue;
        }
        if ($row > -1) {
          tb_megamenu_insert_tb_item($item_config, $row, $col, $item);
        }
        else {
          $row = 0;
          $col = 0;
          while (isset($item_config['rows_content'][$row][$col]['col_content']) && $item_config['rows_content'][$row][$col]['col_content'][0]['type'] == 'block') {
            $row++;
          }
          tb_megamenu_insert_tb_item($item_config, $row, $col, $item);
          $item_config['rows_content'][$row][$col]['col_config'] = array();
        }
      }
    }
  }
}

function tb_megamenu_insert_tb_item(&$item_config, $row, $col, $item) {
  $i = 0;
  $col_content = isset($item_config['rows_content'][$row][$col]['col_content']) ? $item_config['rows_content'][$row][$col]['col_content'] : array();
  while ($i < count($col_content) && $col_content[$i]['weight'] < $item['link']['weight']) {
    $i++;
  }
  for ($j = count($col_content); $j > $i; $j--) {
    $item_config['rows_content'][$row][$col]['col_content'][$j] = $item_config['rows_content'][$row][$col]['col_content'][$j - 1];
  }
  $item_config['rows_content'][$row][$col]['col_content'][$i] = array(
    'mlid' => $item['link']['mlid'],
    'type' => 'menu_item',
    'weight' => $item['link']['weight'],
    'tb_item_config' => array(),
  );
}

function tb_megamenu_array_to_object($array) {
  $obj = new stdClass;
  foreach ($array as $k => $v) {
    if (is_array($v)) {
      $obj->{$k} = tb_megamenu_array_to_object($v);
    }
    else {
      $obj->{$k} = $v;
    }
  }
  return $obj;
}

function tb_megamenu_edit_item_config(&$item_config) {
  if (!isset($item_config['xicon'])) {
    $item_config['xicon'] = "";
  }
  if (!isset($item_config['class'])) {
    $item_config['class'] = "";
  }
  if (!isset($item_config['caption'])) {
    $item_config['caption'] = "";
  }
  if (!isset($item_config['alignsub'])) {
    $item_config['alignsub'] = "";
  }
  if (!isset($item_config['group'])) {
    $item_config['group'] = 0;
  }
  if (!isset($item_config['hidewcol'])) {
    $item_config['hidewcol'] = 0;
  }
  if (!isset($item_config['hidesub'])) {
    $item_config['hidesub'] = 0;
  }
}

function tb_megamenu_edit_submenu_config(&$submenu_config) {
  $submenu_config += array(
    'width' => '',
    'class' => '',
    'group' => ''
  );
}

function tb_megamenu_edit_col_config(&$col_config) {
  $col_config += array(
    'width' => 12,
    'class' => 'class',
    'hidewcol' => 0,
    'showblocktitle' => 0
  );
}

function tb_megamenu_edit_block_config(&$block_config) {
  $block_config += array(
    'animation' => 'none',
    'style' => '',
    'auto-arrow' => TRUE,
    'duration' => 400,
    'delay' => 200,
      'always-show-submenu' => 1
    ) + tb_megamenu_load_block_config_info();
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

function tb_megamenu_animation_options() {
  return array(
    'none' => t('None'),
    'fading' => t('Fading'),
    'slide' => t('Slide'),
    'zoom' => t('Zoom'),
    'elastic' => t('Elastic')
  );
}

function tb_megamenu_style_options() {
  return array(
    '' => t('Default'),
    'blue' => t('Blue'),
    'green' => t('Green')
  ) + tb_megamenu_load_extension_info('theme');
}

function tb_megamenu_find_hook_templates($cache, $path) {
  $templates = array();
  $regex = '/\.tpl\.php$/';

  // Because drupal_system_listing works the way it does, we check for real
  // templates separately from checking for patterns.
  $files = drupal_system_listing($regex, $path, 'name', 0);
  if (isset($files['tb-megamenu-submenu--default.tpl'])) {
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
      $matches = preg_grep('/^' . $pattern . '/', $patterns);
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
  foreach ($no_keys as $key) {
    if (strpos($str, $key) !== false) {
      return false;
    }
  }
  $yes_keys = array('.', '-', ',', '{');
  foreach ($yes_keys as $key) {
    if (strpos($str, $key) !== false) {
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
  foreach ($files as $css_file) {
    $f = fopen($css_file, "r");
    $lines = array();
    $ls = array();
    while ($line = fgets($f)) {
      $lines[] = $line;
      $line = str_replace("\r\n", "", $line);
      if (tb_megamenu_is_a_key($line)) {
        $line = trim($line);
        if (strpos($line, '.tb-megamenu') !== 0) {
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

function tb_megamenu_get_counter($key) {
  $value = &drupal_static($key, 0);
  $value++;
  global $tb_elements_counter;
  if (!$tb_elements_counter) {
    $tb_elements_counter = array();
  }
  $tb_elements_counter[$key] = $value;
  return "tb-megamenu-$key-$value";
}

/**
 * Update items in tb_megamenus table.
 */
function tb_megamenu_update_megamenus(&$form, &$form_state) {
  if (isset($form_state['build_info']['args'][0]['menu_name']) ) {
    $menu_name = $form_state['build_info']['args'][0]['menu_name'];
    if ($menu_config = tb_megamenu_get_menu_config($menu_name)) {
      $items = tb_megamenu_get_tree($menu_name);
      if (function_exists('i18n_menu_localize_tree')) {
        $items = i18n_menu_localize_tree($items);
      }
      $block_config = tb_megamenu_get_block_config($menu_name);
      tb_megamenu_sync_config_all($items, $menu_config, 'backend');
      tb_megamenu_sync_order_menus($menu_config);
      
      global $language;
      $item = db_select('tb_megamenus', 't')
        ->fields('t')
        ->condition('menu_name', $menu_name)
        ->condition('language', $language->language)
        ->execute()->fetchAssoc();

      $record = array(
        'menu_name' => $menu_name,
        'block_config' => json_encode($block_config),
        'menu_config' => json_encode($menu_config),
        'language' => $language->language
      );

      if ($item) {
        drupal_write_record('tb_megamenus', $record, array(
          'menu_name',
          'language'
        ));
      }
      else {
        drupal_write_record('tb_megamenus', $record);
      }
    }
  }
}

/**
 * Get the info of extension.
 *
 * @param $type string
 *  Type of extension.
 *
 * @return array
 */
function tb_megamenu_load_extension_info($type) {
  $datas = &drupal_static(__FUNCTION__ . '_' . $type);
  if (!isset($datas)) {
    $cache_id = ($type == 'theme') ? TB_MEGAMENU_THEME_CACHE_ID : TB_MEGAMENU_EXTENSION_CACHE_ID;
    // Get data from cache.
    if ($cache = cache_get($cache_id)) {
      $datas = $cache->data;
    }
    else {
      // Create caching to store info of extensions.
      $datas = _load_extension_info($type);
      cache_set($cache_id, $datas);
    }
  }
  return $datas;
}

/**
 * Get the info of extension from the declared function (hooks).
 *
 * @param $type string
 *  Type of extension.
 *
 * @return array
 */
function _load_extension_info($type) {
  $datas = array();
  $modules = module_implements('tb_megamenu_extension_info');
  foreach ($modules as $module) {
    $function = $module . '_tb_megamenu_extension_info';
    if (function_exists($function)) {
      $data = $function();
      if (isset($data['type']) && $data['type'] == $type && !empty($data['title'])) {
        $datas[$module] = t($data['title']);
      }
    }
  }
  return $datas;
}

/**
 * Load configuration from modules which implement hook_tb_megamenu_block_config_info.
 *
 * @return array
 */
function tb_megamenu_load_block_config_info() {
  $datas = &drupal_static(__FUNCTION__);
  if (!isset($datas)) {
    // Get data from cache.
    if ($cache = cache_get(TB_MEGAMENU_BLOCK_CONFIG_CACHE_ID)) {
      $datas = $cache->data;
    }
    else {
      $datas = array();
      // Create caching to store info of block configuration.
      $modules = module_implements('tb_megamenu_block_config_info');
      foreach ($modules as $module) {
        $function = $module . '_tb_megamenu_block_config_info';
        if (function_exists($function)) {
          $datas += $function();
        }
      }
      cache_set(TB_MEGAMENU_BLOCK_CONFIG_CACHE_ID, $datas);
    }
  }
  return $datas;
}

/**
 * Init items on Mega toolbox in Backend.
 *
 * @param $configs array Configuration in TB Mega Menu.
 * @return array
 */
function tb_megamenu_init_megatoolbox_item($config) {
  $items = array(
    array(
      'item_attributes' => array(
        'id' => 'tb-megamenu-style-wrapper',
        'title' => t('Select style for this megamenu'),
      ),
      'label' => t('Style'),
      'label_attributes' => array(
        'class' => array('hasTip'),
      ),
      'fieldset_attributes' => array(
        'class' => array('btn-group')
      ),
      'data' => array(
        'tb-megamenu-style' => array(
          '#type' => 'select',
          '#options' => tb_megamenu_style_options(),
          '#default_value' => $config['style'],
          '#attributes' => array('name' => 'tb-megamenu-style')
        )
      ),
    ),
    array(
      'item_attributes' => array(
        'id' => 'tb-megamenu-animation-wrapper',
        'title' => t('Select animation for this megamenu - CSS3 Animation'),
      ),
      'label' => t('Animation'),
      'label_attributes' => array('class' => array('hasTip')),
      'fieldset_attributes' => array(
        'class' => array('btn-group')
      ),
      'data' => array(
        'tb-megamenu-animation' => array(
          '#type' => 'select',
          '#options' => tb_megamenu_animation_options(),
          '#default_value' => $config['animation'],
          '#attributes' => array('name' => 'tb-megamenu-animation')
        )
      )
    ),
    array(
      'item_attributes' => array(
        'id' => 'tb-megamenu-delay-wrapper',
        'title' => t('Delay time of the animation - this field must be an integer.'),
        'style' => 'display:' . ($config['animation'] == 'none' ? 'none' : 'inline-block')
      ),
      'label' => t('Delay (ms)'),
      'label_attributes' => array('class' => array('hasTip')),
      'fieldset_attributes' => array(
        'class' => array('btn-group')
      ),
      'data' => array(
        'tb-megamenu-delay' => array(
          '#type' => 'textfield',
          '#attributes' => array(
            'class' => array('input-medium', 'toolitem-delay', 'toolbox-input'),
            'data-name' => 'delay',
            'name' => 'tb-megamenu-delay'
          ),
          '#value' => $config['delay'] ? $config['delay'] : 0
        )
      )
    ),
    array(
      'item_attributes' => array(
        'id' => 'tb-megamenu-duration-wrapper',
        'title' => t('Duration of the animation - this field must be an integer.'),
        'style' => 'display:' . ($config['animation'] == 'none' ? 'none' : 'inline-block')
      ),
      'label' => t('Duration (ms)'),
      'label_attributes' => array('class' => array('hasTip')),
      'fieldset_attributes' => array(
        'class' => array('btn-group')
      ),
      'data' => array(
        'tb-megamenu-duration' => array(
          '#type' => 'textfield',
          '#attributes' => array(
            'class' => array(
              'input-medium',
              'toolitem-duration',
              'toolbox-input'
            ),
            'data-name' => 'duration',
            'name' => 'tb-megamenu-duration'
          ),
          '#value' => $config['duration'] ? $config['duration'] : 0
        )
      )
    ),
    array(
      'label' => t('Auto arrow'),
      'label_attributes' => array(
        'class' => array('hasTip'),
        'title' => t('Show/hide the arrow next to items that have submenu.')
      ),
      'fieldset_attributes' => array(
        'class' => array('radio', 'btn-group', 'toolitem-auto-arrow'),
        'data-auto-arrow' => $config['auto-arrow']
      ),
      'data' => array(
        '#type' => 'tb_radios',
        '#options' => array(
          array(
            'label' => t('No'),
            'label_attributes' => array(
              'for' => 'toggleAutoArrow0',
              'class' => array(
                'btn',
                $config['auto-arrow'] ? '' : 'active btn-danger'
              ),
              'title' => t('Hide the arrow next to items that have submenu.')
            ),
            'radio_attributes' => array(
              'data-action' => 'toggleAutoArrow',
              'class' => array('toolbox-toggle'),
              'id' => 'toggleAutoArrow0',
              'checked' => $config['auto-arrow'] ? '' : 'checked',
              'value' => 0,
              'name' => 'tb-megamenu-always-show-submenu'
            )
          ),
          array(
            'label' => t('Yes'),
            'label_attributes' => array(
              'for' => 'toggleAutoArrow1',
              'class' => array(
                'btn',
                $config['auto-arrow'] ? 'active btn-success' : ''
              ),
              'title' => t('Hide the arrow next to items that have submenu.')
            ),
            'radio_attributes' => array(
              'data-action' => 'toggleAutoArrow',
              'class' => array('toolbox-toggle'),
              'id' => 'toggleAutoArrow1',
              'checked' => $config['auto-arrow'] ? 'checked' : '',
              'value' => 1,
              'name' => 'tb-megamenu-always-show-submenu'
            )
          )
        )
      )
    ),
    array(
      'label' => t('Always show submenu'),
      'label_attributes' => array(
        'class' => array('hasTip'),
        'title' => t('Show or collapse submenus when browsing on small screens')
      ),
      'fieldset_attributes' => array(
        'class' => array('radio', 'btn-group', 'toolitem-always-show-submenu'),
        'data-always-show-submenu' => $config['always-show-submenu']
      ),
      'data' => array(
        '#type' => 'tb_radios',
        '#options' => array(
          array(
            'label' => t('No'),
            'label_attributes' => array(
              'for' => 'toggleAlwayShowSubmenu0',
              'class' => array(
                'btn',
                $config['always-show-submenu'] ? '' : 'active btn-danger'
              ),
              'title' => t('Hide the arrow next to items that have submenu.')
            ),
            'radio_attributes' => array(
              'data-action' => 'toggleAlwayShowSubmenu',
              'class' => array('toolbox-toggle'),
              'id' => 'toggleAlwayShowSubmenu0',
              'checked' => $config['always-show-submenu'] ? '' : 'checked',
              'value' => 0,
              'name' => 'tb-megamenu-always-show-submenu'
            )
          ),
          array(
            'label' => t('Yes'),
            'label_attributes' => array(
              'for' => 'toggleAlwayShowSubmenu1',
              'class' => array(
                'btn',
                $config['always-show-submenu'] ? 'active btn-success' : ''
              ),
              'title' => t('Hide the arrow next to items that have submenu.')
            ),
            'radio_attributes' => array(
              'data-action' => 'toggleAlwayShowSubmenu',
              'class' => array('toolbox-toggle'),
              'id' => 'toggleAlwayShowSubmenu1',
              'checked' => $config['always-show-submenu'] ? 'checked' : '',
              'value' => 1,
              'name' => 'tb-megamenu-always-show-submenu'
            ),
          ),
        )
      )
    )
  );

  return $items;
}