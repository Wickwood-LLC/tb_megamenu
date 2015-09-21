(function ($) {
  var OffCanvas = {
    prepareToSave: function () {
      return {
        'off_canvas' : {
          'status': $("[name='tb-megamenu-off-canvas']:checked").val(),
          //'config': JSON.stringify({
          //  'example': 'Text text'
          //})
        }
      };
    }
  };

  Drupal.behaviors.OffCanvas = {
    attach: function () {
      // Get data from Off-canvas.
      Drupal.TBMegaMenu.registerExtension({'off_canvas': OffCanvas});
    }
  };
})(jQuery);