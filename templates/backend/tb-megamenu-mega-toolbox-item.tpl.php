<li<?php print drupal_attributes($item_attributes); ?> >
  <label<?php print drupal_attributes($label_attributes); ?>><?php print t($label) ?></label>
  <fieldset<?php print drupal_attributes($fieldset_attributes) ?>>
    <?php print drupal_render($data); ?>
  </fieldset>
</li>