(function ($) {

  var OffCanvas = {
    prepareToSave: function () {
      return {'off-canvas': $("[name='tb-megamenu-off-canvas']:checked").val()};
    }
  };

  Drupal.behaviors.OffCanvas = {
    attach: function () {
      // Get data from Off-canvas.
      Drupal.TBMegaMenu.registerExtension({'off-canvas': OffCanvas});
    }
  };
})(jQuery);