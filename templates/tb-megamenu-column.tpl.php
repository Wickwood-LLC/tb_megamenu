<div <?php print $attributes; ?> class="<?php print $classes; ?>">
  <div class="tb-megamenu-column-inner mega-inner clearfix">
    <?php if (isset($close_button)): ?>
      <?php print $close_button; ?>
    <?php endif; ?>
    <?php print drupal_render($tb_items); ?>
  </div>
</div>
