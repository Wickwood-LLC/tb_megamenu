<li <?php print $attributes;?> class="<?php print $classes;?>">
  <a href="<?php print url($item['link']['href']);?>" class="<?php print implode(" ", $a_classes);?>">
    <?php if(!empty($item_config['xicon'])) : ?>
      <i class="<?php print $item_config['xicon'];?>"></i>
    <?php endif;?>    
    <?php print $item['link']['link_title'];?>
	<?php if($submenu && $block_config['auto-arrow']) :?>
      <span class="caret"></span>
    <?php endif;?>
    <?php if(!empty($item_config['caption'])) : ?>
      <span class="mega-caption"><?php print $item_config['caption'];?></span>
    <?php endif;?>
  </a>
  <?php print $submenu ? $submenu : "";?>
</li>
