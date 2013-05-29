<?php

function tb_megamenu_get_counter($key) {
  $value = &drupal_static($key, 0);
  $value ++;
  global $tb_elements_counter;
  if(!$tb_elements_counter) {
    $tb_elements_counter = array();
  }
  $tb_elements_counter[$key] = $value;
  return "tb-megamenu-$key-$value";
}

function template_preprocess_tb_megamenu(&$vars) {
  $menu_name = $vars['menu_name'];
  $items = menu_tree_all_data($menu_name);
  $items_page = menu_tree_page_data($menu_name);
  $trail = tb_megamenu_build_page_trail($items_page);
  $menu_config = tb_megamenu_get_menu_config($menu_name);
  $block_config = tb_megamenu_get_block_config($menu_name);
  tb_megamenu_sync_config_all($items, $menu_config);
  tb_megamenu_edit_block_config($block_config);
  $vars['classes_array'][] = "tb-megamenu-" . str_replace('_', '-', $menu_name);
  if($vars['section'] == 'frontend' && $block_config['style'] != '') {
    drupal_add_css(drupal_get_path("module", "tb_megamenu") . "/css/styles/" . $block_config['style'] . ".css");
    $vars['classes_array'][] = "tb-megamenu-" . $block_config['style'];
  }
  if($block_config['animation'] != 'none') {
    $vars['classes_array'][] = "animate";
    $vars['classes_array'][] = $block_config['animation'];
    $vars['attributes_array']['data-duration'] = $block_config['duration'];
  }
  $vars['content'] = theme('tb_megamenu_ul', array(
    'level' => 0,
    'menu_config' => $menu_config,
    'block_config' => $block_config,
    'items' => $items,
    'trail' => $trail,
    'section' => $vars['section'],
  ));
  $vars['block_config'] = $block_config;
  if($block_config['animation'] != 'none') {
    $css = array();
    $css[] = '.tb-megamenu.animate .mega > .mega-dropdown-menu, .tb-megamenu.animate.slide .mega > .mega-dropdown-menu > div {';
    $css[] = '  transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '  -webkit-transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '  -ms-transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '  -o-transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '}';
    drupal_add_css(implode("\n", $css), array('type' => 'inline'));
  }
}

function template_preprocess_tb_megamenu_ul(&$vars) {
  $items = $vars['items'];
  $level = $vars['level'];
  $lis = array();
  foreach ($items as $item) {
    if(!$item['link']['hidden']) {
      $lis[] = theme('tb_megamenu_li', array(
        'level' => $level + 1,
        'item' => $item,
        'menu_config' => $vars['menu_config'],
        'block_config' => $vars['block_config'],
        'trail' => $vars['trail'],
        'section' => $vars['section'],
      ));
    }
  }
  $vars['lis'] = implode("\n", $lis);
  $vars['classes_array'][] = "nav";
  $vars['classes_array'][] = "level" . $level;
  $vars['classes_array'][] = "tb-megamenu-items-" . count($items);
}

function template_preprocess_tb_megamenu_li(&$vars) {
  $level = $vars['level'];
  $item = $vars['item'];
  $mlid = $item['link']['mlid'];
  $trail = $vars['trail'];
  $menu_config = $vars['menu_config'];
  if((isset($menu_config[$mlid]['rows_content']) && !empty($menu_config[$mlid]['rows_content'])) || !empty($item['below'])) {
    $vars['submenu'] = theme('tb_megamenu_submenu', array(
      'parent' => $item,
      'level' => $level,
      'menu_config' => $vars['menu_config'],
      'block_config' => $vars['block_config'],
      'trail' => $vars['trail'],
      'section' => $vars['section'],
    ));
  }
  $item_config = isset($menu_config[$mlid]['item_config']) ? $menu_config[$mlid]['item_config'] : array();
  tb_megamenu_edit_item_config($item_config);
  $vars['classes_array'][] = 'tb-megamenu-level-' . $level;
  $vars['classes_array'][] = 'mega';
  if(!empty($item_config['class'])) {
    $vars['classes_array'][] = $item_config['class'];
  }
  if(!empty($item_config['alignsub'])) {
    $vars['classes_array'][] = "mega-align-" . $item_config['alignsub'];
  }
  if($item_config['hidewcol']) {
    $vars['classes_array'][] = "sub-hidden-collapse";
  }
  if ($vars['section'] == 'frontend' && $item_config['hidesub']) {
    $vars['submenu'] = NULL;
  }

  if($item_config['group'] && $vars['submenu']) {
    $vars['classes_array'][] = "mega-group";
    $vars['a_classes'][] = 'mega-group-title';
  }
  elseif($vars['submenu'] && !$item_config['hidesub']) {
    $vars['a_classes'][] = 'dropdown-toggle';
    if($level == 1) {
      $vars['classes_array'][] = 'dropdown';
    }
    else {
      $vars['classes_array'][] = 'dropdown-submenu';
    }
  }

  if(isset($trail[$mlid])) {
    $vars['classes_array'][] = 'active';
  }
  $vars['attributes_array']['data-id'] = $mlid;
  $vars['attributes_array']['data-level'] = $level;
  $vars['attributes_array']['data-type'] = 'menu_item';
  $vars['attributes_array']['data-class'] = $item_config['class'];
  $vars['attributes_array']['data-xicon'] = $item_config['xicon'];
  $vars['attributes_array']['data-caption'] = $item_config['caption'];
  $vars['attributes_array']['data-alignsub'] = $item_config['alignsub'];
  $vars['attributes_array']['data-group'] = $item_config['group'];
  $vars['attributes_array']['data-hidewcol'] = $item_config['hidewcol'];
  $vars['attributes_array']['data-hidesub'] = $item_config['hidesub'];
  if($vars['section'] == 'backend') {
    $vars['attributes_array']['title'] = t("Menu Item");
    if(!$item_config['hidesub'] && $vars['submenu']) {
      $vars['attributes_array']['title'] .= " - " . t("Click to keep submenu of this item showed.");
    }
  }
  
  $vars['item_config'] = $item_config;
}

function template_preprocess_tb_megamenu_submenu(&$vars) {
  $parent = $vars['parent'];
  $mlid = $parent['link']['mlid'];
  $menu_config = $vars['menu_config'];
  $item_config = isset($menu_config[$mlid]) ? $menu_config[$mlid] : array();
  $submenu_config = isset($item_config['submenu_config']) ? $item_config['submenu_config'] : array();
  tb_megamenu_edit_submenu_config($submenu_config);
  if(!(empty($item_config))) {
    $rows = array();
    foreach($item_config['rows_content'] as $row) {
      $rows[] = theme('tb_megamenu_row', array(
        'row' => $row,
        'parent' => $vars['parent'],
        'level' => $vars['level'],
        'menu_config' => $vars['menu_config'],
        'block_config' => $vars['block_config'],
        'trail' => $vars['trail'],
        'section' => $vars['section'],
      ));
    }
    $vars['rows'] = implode("\n", $rows);
    $vars['submenu_config'] = $submenu_config;
    if($submenu_config['class'] != "") {
      $vars['classes_array'][] = $submenu_config['class'];
    }
    if($submenu_config['group']) {
      $vars['classes_array'][] = 'mega-group-ct';
    }
    else {
      $vars['classes_array'][] = 'dropdown-menu';
      $vars['classes_array'][] = 'mega-dropdown-menu';
    }
    $vars['classes_array'][] = 'nav-child';
    $vars['attributes_array']['data-class'] = $submenu_config['class'];
    $vars['attributes_array']['data-width'] = $submenu_config['width'];
    if (!empty($submenu_config['width'])) {
      $vars['attributes_array']['style'] = "width: " . $submenu_config['width'] . "px;";
    }
    if($vars['section'] == 'backend') {
      $vars['attributes_array']['title'] = t("Submenu - Click to config for this submenu");
    }
  }
}

function template_preprocess_tb_megamenu_row(&$vars) {
  $row = $vars['row'];
  $cols = array();
  foreach($row as $col) {
    $cols[] = theme('tb_megamenu_column', array(
      'col' => $col,
      'parent' => $vars['parent'],
      'level' => $vars['level'],
      'menu_config' => $vars['menu_config'],
      'block_config' => $vars['block_config'],
      'trail' => $vars['trail'],
      'section' => $vars['section'],
    ));
  }
  $vars['columns'] = implode("\n", $cols);
}

function template_preprocess_tb_megamenu_column(&$vars) {
  $col = $vars['col']['col_content'];
  $col_config = $vars['col']['col_config'];
  tb_megamenu_edit_col_config($col_config);
  $vars['close_button'] = FALSE;
  $tb_items = array();
  foreach($col as $tb_item) {
    if($tb_item['type'] == 'menu_item') {
      $tb_items[] = theme('tb_megamenu_subul', array(
        'col' => $col,
        'items' => $vars['parent']['below'],
        'level' => $vars['level'],
        'menu_config' => $vars['menu_config'],
        'block_config' => $vars['block_config'],
        'trail' => $vars['trail'],
        'section' => $vars['section'],
      ));
      break;
    }
    else {
      $tb_items[] = theme('tb_megamenu_block', array(
        'bid' => $tb_item['bid'],
      ));
      if ($vars['section'] !== 'frontend') {
        $vars['close_button'] = '<span class="close" title="' . t("Remove this block") . '">X</span>';
      }
    }
  }
  $vars['tb_items'] = implode("\n", $tb_items);
  $vars['col_config'] = $col_config;
  $vars['classes_array'][] = 'span' . $col_config['width'];
  $vars['classes_array'][] = ' mega-col-nav';
  if(!empty($col_config['class'])) {
    $vars['classes_array'][] = $col_config['class'];
  }
  if(!empty($col_config['hidewcol'])) {
    $vars['classes_array'][] = $col_config['class'];
  }
  if($col_config['hidewcol']) {
    $vars['classes_array'][] = 'hidden-collapse';
  }
  $vars['attributes_array']['data-class'] = $col_config['class'];
  $vars['attributes_array']['data-width'] = $col_config['width'];
  $vars['attributes_array']['data-hidewcol'] = $col_config['hidewcol'];
  $vars['attributes_array']['id'] = tb_megamenu_get_counter("column");
  if($vars['section'] == 'backend') {
    $vars['attributes_array']['title'] = t("Column - Click to config for this column");
  }
}

function template_preprocess_tb_megamenu_subul(&$vars) {
  $col = $vars['col'];
  $items = $vars['items'];
  $level = $vars['level'];
  $lis = array();
  foreach($col as $tb_item) {
    $item = tb_megamenu_get_menu_item('main-menu', $tb_item['mlid']);
    $lis[] = theme('tb_megamenu_li', array(
      'level' => $level + 1,
      'item' => $item,
      'menu_config' => $vars['menu_config'],
      'block_config' => $vars['block_config'],
      'trail' => $vars['trail'],
      'section' => $vars['section'],
    ));
  }
  $vars['lis'] = implode("\n", $lis);
  $vars['classes_array'][] = "mega-nav";
  $vars['classes_array'][] = "level" . $level;
  $vars['classes_array'][] = "tb-megamenu-items-" . count($col);
}

function template_preprocess_tb_megamenu_block(&$vars) {
  $block = tb_megamenu_load_block($vars['bid']);
  if($block) {
    $module = $block['module'];
    $delta = $block['delta'];
    $block = tb_megamenu_array_to_object($block);
    $content = module_invoke($module, 'block_view', $delta);
    $block->subject = empty($content['subject']) ? $block->info : $content['subject'];
    if($content) {
      $vars['content'] = theme("block", array('elements' => array('content' => $content['content'], '#children' => (is_array($content['content']) ? drupal_render($content['content']) : $content['content']), '#block' => $block)));
    }
    $vars['classes_array'][] = "tb-block";
    $vars['classes_array'][] = "tb-megamenu-block";
    $vars['attributes_array']['data-type'] = "block";
    $vars['attributes_array']['data-id'] = $block->bid;
  }
  else {
    $vars['content'] = NULL;
  }
}

function template_preprocess_tb_megamenu_backend(&$vars) {
  $menu_name = $vars['menu_name'];
  $block_config = tb_megamenu_get_block_config($menu_name);
  tb_megamenu_edit_block_config($block_config);
  $menu_content = tb_megamenu_render_block($menu_name);
  $vars['menu_content'] = $menu_content['content']['#markup'];
  $vars['item_toolbox'] = theme_render_template(drupal_get_path('module', 'tb_megamenu') . "/templates/backend/tb-megamenu-item-toolbox.tpl.php", array());
  $vars['submenu_toolbox'] = theme_render_template(drupal_get_path('module', 'tb_megamenu') . "/templates/backend/tb-megamenu-submenu-toolbox.tpl.php", array());
  $vars['column_toolbox'] = theme_render_template(drupal_get_path('module', 'tb_megamenu') . "/templates/backend/tb-megamenu-column-toolbox.tpl.php", array('blocks' => tb_megamenu_get_blocks_info()));
  $vars['block_config'] = $block_config;
  $vars['animation_options'] = tb_megamenu_animation_options($block_config);
  $vars['style_options'] = tb_megamenu_style_options($block_config);
  $vars['edit_links'] = base_path() . "admin/structure/menu/manage/$menu_name";
  $vars['edit_menu'] = base_path() . "admin/structure/menu/manage/$menu_name/edit";
  
  $js = 'Drupal.TBMegaMenu = Drupal.TBMegaMenu || {};
  (function ($) {
    Drupal.behaviors.tbMegaMenuAction = {
      attach: function(context) {
        $("#tb-megamenu-admin-mm-container").megamenuAdmin({menu_name: "' . $menu_name . '"});
      }
    }
  })(jQuery);
  ';
  drupal_add_js($js, array('type' => 'inline'));
}
