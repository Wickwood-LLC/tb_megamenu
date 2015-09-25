<?php if ($content) : ?>
  <div <?php print $attributes; ?> class="<?php print $classes; ?>">
    <div class="block-inner">
      <?php print drupal_render($content); ?>
    </div>
  </div>
<?php endif; ?>
