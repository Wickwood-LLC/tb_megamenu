(function ($) {
  var OffCanvas = {
    prepareToSave: function () {
      var offCanvasItem = $('.tb-megamenu ul[class*="level"] > .off-canvas-item');
      var offCanvasConfig = Drupal.TBMegaMenu.getItemData(offCanvasItem);      
      return {
        'off_canvas' : {
          'status': $("[name='tb-megamenu-off-canvas']:checked").val(),
          'config': JSON.stringify(offCanvasConfig)
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