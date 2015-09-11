<div id="tb-megamenu-admin" class="hidden tb-megamenu-admin tb-megamenu">
  <div class="admin-inline-toolbox clearfix">
    <div class="tb-megamenu-admin-mm-row clearfix">
      <div id="tb-megamenu-admin-mm-tb">
        <div id="toolbox-loading" class="toolbox-loading">&nbsp;</div>
        <div id="toolbox-message" class="toolbox-message">&nbsp;</div>
        <div id="tb-megamenu-admin-mm-intro" class="pull-left admin-toolbox">
          <h3><?php print t('MegaMenu Toolbox') ?></h3>
          <p><?php print t('This toolbox includes all settings of megamenu, just select menu then configure. There are 3 level of configuration: sub-megamenu setting, column setting and menu item setting.') ?></p>
          <ul>
            <?php foreach ($mega_toolbox_items as $item) : ?>
              <?php print theme('tb_megamenu_mega_toolbox_item', $item); ?>
            <?php endforeach; ?>
          </ul>
        </div>
        <?php print $item_toolbox;?>
        <?php print $submenu_toolbox;?>
        <?php print $column_toolbox;?>
      </div>
      <div class="toolbox-actions-group">
        <button class="btn btn-success toolbox-action toolbox-saveConfig" data-action="saveConfig"><i class="fa fa-save"></i> <?php print t('Save') ?></button>
        <button class="btn btn-danger toolbox-action toolbox-resetConfig" data-action="resetConfig"><i class="fa fa-undo"></i> <?php print t('Reset') ?></button>
      </div>
      <div class="toolbox-links-groups">
        <a href="<?php print $edit_menu;?>" target="_blank">Edit menu</a>
        <a href="<?php print $edit_links;?>" target="_blank">Edit links</a>
      </div>
    </div>
  </div>

  <div id="tb-megamenu-admin-mm-container" class="navbar clearfix">
    <?php print $menu_content;?>
  </div>
</div>
