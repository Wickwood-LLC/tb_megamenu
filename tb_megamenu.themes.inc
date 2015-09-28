<?php

function template_preprocess_tb_megamenu(&$vars) {
  $menu_name = $vars['menu_name'];
  if ($vars['section'] == 'backend') {
    $items = tb_megamenu_get_tree($menu_name);
  }
  else {
    $items = menu_tree_all_data($menu_name);
  }
  if (function_exists('i18n_menu_localize_tree')) {
    $items = i18n_menu_localize_tree($items);
  }
  $items_page = menu_tree_page_data($menu_name);
  $trail = tb_megamenu_build_page_trail($items_page);
  $menu_config = tb_megamenu_get_menu_config($menu_name);
  $block_config = tb_megamenu_get_block_config($menu_name);
  tb_megamenu_sync_config_all($items, $menu_config, $vars['section']);
  tb_megamenu_edit_block_config($block_config);
  $vars['classes_array'][] = "tb-megamenu-" . str_replace('_', '-', $menu_name);
  if ($vars['section'] == 'frontend' && $block_config['style'] != '') {
    drupal_add_css(drupal_get_path("module", "tb_megamenu") . "/css/styles/" . $block_config['style'] . ".css", array(
      'basename' => "tb-megamenu-style-" . $block_config['style'] . ".css",
    ));
    $vars['classes_array'][] = "style-" . $block_config['style'];
  }

  if ($block_config['animation'] != 'none') {
    $vars['classes_array'][] = "animate";
    $vars['classes_array'][] = $block_config['animation'];
    $vars['attributes_array']['data-duration'] = $block_config['duration'];
    $vars['attributes_array']['data-delay'] = $block_config['delay'];
    $vars['attributes_array']['id'] = "tb-megamenu-" . str_replace('_', '-', $menu_name);
  }

  $vars['block_config'] = $block_config;
  if ($vars['section'] == 'frontend' && $block_config['animation'] != 'none') {
    $css = array();
    $css[] = '.tb-megamenu.animate .mega > .mega-dropdown-menu, .tb-megamenu.animate.slide .mega > .mega-dropdown-menu > div {';
    $css[] = '  transition-delay: ' . $block_config['delay'] . 'ms;';
    $css[] = '  -webkit-transition-delay: ' . $block_config['delay'] . 'ms;';
    $css[] = '  -ms-transition-delay: ' . $block_config['delay'] . 'ms;';
    $css[] = '  -o-transition-delay: ' . $block_config['delay'] . 'ms;';
    $css[] = '  transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '  -webkit-transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '  -ms-transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '  -o-transition-duration: ' . $block_config['duration'] . 'ms;';
    $css[] = '}';
    drupal_add_css(implode("\n", $css), array('type' => 'inline'));
  }

  // Get selected style.
  $selected_style = '';
  $styles = array_keys(tb_megamenu_load_extension_info('theme'));
  if (in_array($block_config['style'], $styles)) {
    $selected_style = $block_config['style'];
  }
  // Get info of extension.
  $extensions = tb_megamenu_load_extension_info('extension');

  $vars['content'] = array(
    '#theme' => 'tb_megamenu_nav',
    '#menu_name' => $vars['menu_name'],
    '#level' => 0,
    '#menu_config' => $menu_config,
    '#block_config' => $block_config,
    '#items' => $items,
    '#trail' => $trail,
    '#section' => $vars['section'],
    '#selected_style' => $selected_style,
    '#extensions' => $extensions
  );

  // Load preprocess function from the selected style.
  _execute_preprocess_function($selected_style . '_preprocess_tb_megamenu', $vars);
  // Load preprocess functions from the selected extensions.
  foreach ($extensions as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu', $vars);
  }
}

function template_preprocess_tb_megamenu_nav(&$vars) {
  $level = $vars['level'];
  $items = $vars['items'];
  $block_config = $vars['block_config'];
  $vars['classes_array'][] = "nav";
  $vars['classes_array'][] = "level-" . $level;
  $vars['classes_array'][] = "items-" . count($items);

  $vars['lis'] = array();
  foreach ($items as $item) {
    if (!$item['link']['hidden']) {
      $vars['lis'][] = array(
        '#theme' => 'tb_megamenu_item',
        '#menu_name' => $vars['menu_name'],
        '#level' => $vars['level'] + 1,
        '#item' => $item,
        '#menu_config' => $vars['menu_config'],
        '#block_config' => $vars['block_config'],
        '#trail' => $vars['trail'],
        '#section' => $vars['section']
      );
    }
  }

  $styles = array_keys(tb_megamenu_load_extension_info('theme'));
  if (in_array($block_config['style'], $styles)) {
    // Selected extension.
    $selected_style = $block_config['style'];
    _execute_preprocess_function($selected_style . '_preprocess_tb_megamenu_nav', $vars);
  }

  // Load preprocess functions from the selected extensions.
  $extensions = tb_megamenu_load_extension_info('extension');
  foreach ($extensions as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu_nav', $vars);
  }
}

function template_preprocess_tb_megamenu_item(&$vars) {
  $level = $vars['level'];
  $item = $vars['item'];
  $mlid = $item['link']['mlid'];
  $trail = $vars['trail'];
  $link_attributes = array();
  $menu_config = $vars['menu_config'];
  $item_config = isset($menu_config[$mlid]['item_config']) ? $menu_config[$mlid]['item_config'] : array();
  tb_megamenu_edit_item_config($item_config);
  $vars['item_config'] = $item_config;
  if (!empty($menu_config[$mlid]['rows_content']) &&
    ($vars['section'] == 'backend' || !$item_config['hidesub'])
  ) {

    $vars['submenu'] = array(
      '#theme' => 'tb_megamenu_submenu',
      '#menu_name' => $vars['menu_name'],
      '#parent' => $item,
      '#level' => $level,
      '#menu_item_config' => $menu_config[$mlid],
      '#menu_config' => $vars['menu_config'],
      '#block_config' => $vars['block_config'],
      '#trail' => $vars['trail'],
      '#section' => $vars['section'],
      '#selected_style' => $vars['selected_style'],
      '#extensions' => $vars['extensions']
    );
  }
  else {
    $vars['submenu'] = NULL;
  }

  $vars['classes_array'][] = 'level-' . $level;
  $vars['classes_array'][] = 'mega';
  if (!empty($item_config['class'])) {
    $vars['classes_array'][] = $item_config['class'];
  }
  if (!empty($item_config['alignsub'])) {
    $vars['classes_array'][] = "mega-align-" . $item_config['alignsub'];
  }
  if ($item_config['hidewcol']) {
    $vars['classes_array'][] = "sub-hidden-collapse";
  }

  if ($item_config['group'] && $vars['submenu']) {
    $vars['classes_array'][] = "mega-group";
    $link_attributes['class'][] = 'mega-group-title';
  }
  elseif ($vars['submenu'] && !$item_config['hidesub']) {
    $link_attributes['class'][] = 'dropdown-toggle';
    if ($level == 1) {
      $vars['classes_array'][] = 'dropdown';
    }
    else {
      $vars['classes_array'][] = 'dropdown-submenu';
    }
  }

  $vars['item']['link']['#attributes'] = $link_attributes;
  if (!empty($item['link']['localized_options']['attributes'])) {
    if (!(isset($item['link']['localized_options']['attributes']['title']) &&
      $item['link']['localized_options']['attributes']['title'])
    ) {
      $item['link']['localized_options']['attributes']['title'] = $item['link']['title'];
    }
    $vars['item']['link']['#attributes'] += $item['link']['localized_options']['attributes'];
  }

  if (isset($trail[$mlid])) {
    $vars['classes_array'][] = 'active';
    $vars['classes_array'][] = $vars['a_classes'][] = 'active-trail';
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
  if ($vars['section'] == 'backend') {
    $vars['attributes_array']['title'] = t("Menu Item");
  }

  // Load preprocess function from the selected style.
  if ($vars['selected_style']) {
    _execute_preprocess_function($vars['selected_style'] . '_preprocess_tb_megamenu_item', $vars);
  }

  // Load preprocess function from the selected extensions.
  foreach ($vars['extensions'] as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu_item', $vars);
  }
}

function template_preprocess_tb_megamenu_submenu(&$vars) {
//  $mlid = $parent['link']['mlid'];
//  $menu_config = $vars['menu_config'];
//  $item_config = isset($menu_config[$mlid]) ? $menu_config[$mlid] : array();
  $item_config = $vars['menu_item_config'];
  $submenu_config = isset($item_config['submenu_config']) ? $item_config['submenu_config'] : array();
  $vars['submenu_config'] = $submenu_config;
  tb_megamenu_edit_submenu_config($submenu_config);
  $vars['rows'] = array();
  foreach ($item_config['rows_content'] as $row) {
    $vars['rows'][] = array(
      '#theme' => 'tb_megamenu_row',
      '#menu_name' => $vars['menu_name'],
      '#row' => $row,
      '#parent' => $vars['parent'],
      '#level' => $vars['level'],
      '#menu_config' => $vars['menu_config'],
      '#block_config' => $vars['block_config'],
      '#trail' => $vars['trail'],
      '#section' => $vars['section'],
      '#selected_style' => $vars['selected_style'],
      '#extensions' => $vars['extensions']
    );
  }
  if ($submenu_config['class'] != "") {
    $vars['classes_array'][] = $submenu_config['class'];
  }
  if ($submenu_config['group']) {
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

  // Load preprocess function from the selected style.
  if ($vars['selected_style']) {
    _execute_preprocess_function($vars['selected_style'] . '_preprocess_tb_megamenu_submenu', $vars);
  }

  // Load preprocess function from the selected extensions.
  foreach ($vars['extensions'] as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu_submenu', $vars);
  }
}

function template_preprocess_tb_megamenu_row(&$vars) {
  $row = $vars['row'];
  $vars['columns'] = array();
  foreach ($row as $col) {
    $vars['columns'][] = array(
      '#theme' => 'tb_megamenu_column',
      '#menu_name' => $vars['menu_name'],
      '#col' => $col,
      '#parent' => $vars['parent'],
      '#level' => $vars['level'],
      '#menu_config' => $vars['menu_config'],
      '#block_config' => $vars['block_config'],
      '#trail' => $vars['trail'],
      '#section' => $vars['section'],
      '#selected_style' => $vars['selected_style'],
      '#extensions' => $vars['extensions']
    );
  }

  $vars['classes_array'][] = 'row-fluid';
  // Load preprocess function from the selected style.
  if ($vars['selected_style']) {
    _execute_preprocess_function($vars['selected_style'] . '_preprocess_tb_megamenu_row', $vars);
  }

  // Load preprocess function from the selected extensions.
  foreach ($vars['extensions'] as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu_row', $vars);
  }
}

function template_preprocess_tb_megamenu_column(&$vars) {
  $col = $vars['col']['col_content'];
  $col_config = $vars['col']['col_config'];
  tb_megamenu_edit_col_config($col_config);
  $vars['col_config'] = $col_config;
  $vars['tb_items'] = array();
  foreach ($col as $tb_item) {
    if ($tb_item['type'] == 'menu_item') {
      $vars['tb_items'][] = array(
        '#theme' => 'tb_megamenu_subnav',
        '#menu_name' => $vars['menu_name'],
        '#col' => $col,
        '#items' => $vars['parent']['below'],
        '#level' => $vars['level'],
        '#menu_config' => $vars['menu_config'],
        '#block_config' => $vars['block_config'],
        '#trail' => $vars['trail'],
        '#section' => $vars['section'],
        '#selected_style' => $vars['selected_style'],
        '#extensions' => $vars['extensions']
      );
      break;
    }
    else {
      $vars['tb_items'][] = array(
        '#theme' => 'tb_megamenu_block',
        '#block_key' => $tb_item['block_key'],
        '#section' => $vars['section'],
        '#showblocktitle' => $col_config['showblocktitle'],
        '#selected_style' => $vars['selected_style'],
        '#extensions' => $vars['extensions']
      );
      $vars['attributes_array']['data-showblocktitle'] = $col_config['showblocktitle'];
      if ($vars['section'] !== 'frontend') {
        $vars['close_button'] = '<span class="close fa fa-trash-o" title="' . t("Remove this block") . '">&nbsp;</span>';
      }
    }
  }

  $vars['classes_array'][] = 'span' . $col_config['width'];
  $vars['classes_array'][] = ' mega-col-nav';
  if (!empty($col_config['class'])) {
    $vars['classes_array'][] = $col_config['class'];
  }
  if (!empty($col_config['hidewcol'])) {
    $vars['classes_array'][] = $col_config['class'];
  }
  if ($col_config['hidewcol']) {
    $vars['classes_array'][] = 'hidden-collapse';
  }
  $vars['attributes_array']['data-class'] = $col_config['class'];
  $vars['attributes_array']['data-width'] = $col_config['width'];
  $vars['attributes_array']['data-hidewcol'] = $col_config['hidewcol'];
  $vars['attributes_array']['id'] = tb_megamenu_get_counter("column");

  // Load preprocess function from the selected style.
  if ($vars['selected_style']) {
    _execute_preprocess_function($vars['selected_style'] . '_preprocess_tb_megamenu_column', $vars);
  }

  // Load preprocess function from the selected extensions.
  foreach ($vars['extensions'] as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu_column', $vars);
  }
}

function template_preprocess_tb_megamenu_subnav(&$vars) {
  $col = $vars['col'];
  $level = $vars['level'];
  $vars['lis'] = array();
  foreach ($col as $tb_item) {
    $item = tb_megamenu_get_menu_item($vars['menu_name'], $tb_item['mlid']);
    $vars['lis'][] = array(
      '#theme' => 'tb_megamenu_item',
      '#menu_name' => $vars['menu_name'],
      '#level' => $level + 1,
      '#item' => $item,
      '#menu_config' => $vars['menu_config'],
      '#block_config' => $vars['block_config'],
      '#trail' => $vars['trail'],
      '#section' => $vars['section'],
      '#selected_style' => $vars['selected_style'],
      '#extensions' => $vars['extensions']
    );
  }
  $vars['classes_array'][] = "mega-nav";
  $vars['classes_array'][] = "level-" . $level;
  $vars['classes_array'][] = "items-" . count($col);

  // Load preprocess function from the selected style.
  if ($vars['selected_style']) {
    _execute_preprocess_function($vars['selected_style'] . '_preprocess_tb_megamenu_subnav', $vars);
  }

  // Load preprocess function from the selected extensions.
  foreach ($vars['extensions'] as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu_subnav', $vars);
  }
}

function template_preprocess_tb_megamenu_block(&$vars) {
  $block = tb_megamenu_load_block($vars['block_key']);
  if ($block) {
    $module = $block->module;
    $delta = $block->delta;
    $block_content = module_invoke($module, 'block_view', $delta);
    if (empty($block_content) && $vars['section'] == 'backend') {
      // Using description to show instead content.
      $blocks_info = module_invoke($module, 'block_info');
      $vars['content'] = $blocks_info[$delta]['info'];
    }
    elseif ($block_content) {
      $block->subject = '';
      $is_enabled_i18n_block = module_enable(array('i18n_block'));
      // Translate content in block. And content must be a string.
      if ($is_enabled_i18n_block && !is_array($block_content['content'])) {
        $block->content = &$block_content['content'];
        $block->content = i18n_string_text("blocks:$block->module:$block->delta:body", $block->content);
      }
      // Check configuration for show block title;
      if (isset($vars['showblocktitle']) && $vars['showblocktitle']) {
        // If i18n_block is not enabled, so we will set title to subject.
        if (!$is_enabled_i18n_block) {
          $block->subject = $block->title ? $block->title : $block_content['subject'];
        }
        elseif (!empty($block->title) && $block->title != '<none>') {
          // Check plain here to allow module generated titles to keep any markup.
          $block->subject = $block->title;
          $block->subject = i18n_string_plain("blocks:$block->module:$block->delta:title", $block->subject);
        }
      }
      $vars['content'] = array(
        '#theme' => 'block',
        '#children' => (is_array($block_content['content']) ? drupal_render($block_content['content']) : $block_content['content']),
        '#block' => $block
      );
    }
    else {
      $vars['content'] = NULL;
    }
    $vars['classes_array'][] = "tb-block";
    $vars['classes_array'][] = "tb-megamenu-block";
    $vars['attributes_array']['data-type'] = "block";
    $vars['attributes_array']['data-block'] = $vars['block_key'];
  }
  else {
    $vars['content'] = NULL;
  }

  // Load preprocess function from the selected style.
  if ($vars['selected_style']) {
    _execute_preprocess_function($vars['selected_style'] . '_preprocess_tb_megamenu_block', $vars);
  }

  // Load preprocess function from the selected extensions.
  foreach ($vars['extensions'] as $module => $title) {
    _execute_preprocess_function($module . '_preprocess_tb_megamenu_block', $vars);
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
  $vars['column_toolbox'] = theme_render_template(drupal_get_path('module', 'tb_megamenu') . "/templates/backend/tb-megamenu-column-toolbox.tpl.php", array('blocks_options' => tb_megamenu_get_blocks_options()));
  $vars['block_config'] = $block_config;
  $vars['animation_options'] = tb_megamenu_animation_options($block_config);
  $vars['edit_links'] = base_path() . "admin/structure/menu/manage/$menu_name";
  $vars['edit_menu'] = base_path() . "admin/structure/menu/manage/$menu_name/edit";
  // Init items configuration in mega toolbox.
  $vars['mega_toolbox_items'] = tb_megamenu_init_megatoolbox_item($block_config);

  $js_inline = 'Drupal.TBMegaMenu = Drupal.TBMegaMenu || {};
  (function ($) {
    Drupal.behaviors.tbMegaMenuAction = {
      attach: function(context) {
        $("#tb-megamenu-admin-mm-container").megamenuAdmin({menu_name: "' . $menu_name . '"});
      }
    }
  })(jQuery);';
  drupal_add_js($js_inline, array('type' => 'inline'));

  $modules = array_keys(tb_megamenu_load_extension_info('extension'));
  foreach ($modules as $module) {
    $name_function = $module . '_preprocess_tb_megamenu_backend';
    if (function_exists($name_function)) {
      $name_function($vars);
    }
  }
}

/**
 * Theme function for tb_radios.
 */
function theme_tb_radios($vars) {
  $output = '';
  if (count($vars['element']['#options'])) {
    foreach ($vars['element']['#options'] as $option) {
      $option['radio_attributes']['type'] = 'radio';
      $output .= '<input' . drupal_attributes($option['radio_attributes']) . ' />';
      $output .= '<label' . drupal_attributes($option['label_attributes']) . ' />' . $option['label'] . '</label>';
    }
  }
  return $output;
}
