Drupal.TBMegaMenu = Drupal.TBMegaMenu || {};

(function ($) {
  Drupal.TBMegaMenu.menuInstance = false;
  Drupal.TBMegaMenu.supportedScreens = [980];
  Drupal.TBMegaMenu.menuResponsive = function () {
    var windowWidth = window.innerWidth ? window.innerWidth : $(window).width();
    if (windowWidth < Drupal.TBMegaMenu.supportedScreens[0]) {
      $('.tb-megamenu').children('.nav-collapse').addClass('collapse');
    } else {
      var navCollapse = $('.tb-megamenu').children('.nav-collapse');
      navCollapse.removeClass('collapse');
      if (navCollapse.height() <= 0) {
        navCollapse.css({height: 'auto', overflow: 'visible'});
      }
    }
  };
  Drupal.TBMegaMenu.oldWindowWidth = 0;
  Drupal.behaviors.tbMegaMenuAction = {
    attach: function(context) {
      $('.tb-megamenu-button', context).once('menuIstance', function () {
        var This = this;
        $(This).click(function() {
          if(parseInt($(this).parent().children('.nav-collapse').height())) {
            $(this).parent().children('.nav-collapse').css({height: 0, overflow: 'hidden'});
          }
          else {
            $(this).parent().children('.nav-collapse').css({height: 'auto', overflow: 'visible'});
          }
        });
      });
      
      
      var isTouch = 'ontouchstart' in window && !(/hp-tablet/gi).test(navigator.appVersion);
      if(!isTouch){
        $(document).ready(function($){
          var mm_duration = 0;
          $('.tb-megamenu').each (function(){
            if ($(this).data('duration')) {
              mm_duration = $(this).data('duration');
            }
          });
          var mm_timeout = mm_duration ? 100 + mm_duration : 500;
          $('.nav > li, li.mega').hover(function(event) {
            var $this = $(this);
            if ($this.hasClass ('mega')) {
              $this.addClass ('animating');
              clearTimeout ($this.data('animatingTimeout'));
              $this.data('animatingTimeout', setTimeout(function(){$this.removeClass ('animating')}, mm_timeout));
              clearTimeout ($this.data('hoverTimeout'));
              $this.data('hoverTimeout', setTimeout(function(){$this.addClass ('open')}, 100));  
            } else {
              clearTimeout ($this.data('hoverTimeout'));
              $this.data('hoverTimeout', 
              setTimeout(function(){$this.addClass ('open')}, 100));
            }
          },
          function(event) {
            var $this = $(this);
            if ($this.hasClass ('mega')) {
              $this.addClass ('animating');
              clearTimeout ($this.data('animatingTimeout'));
              $this.data('animatingTimeout', 
              setTimeout(function(){$this.removeClass ('animating')}, mm_timeout));
              clearTimeout ($this.data('hoverTimeout'));
              $this.data('hoverTimeout', setTimeout(function(){$this.removeClass ('open')}, 100));
            } else {
              clearTimeout ($this.data('hoverTimeout'));
              $this.data('hoverTimeout', 
              setTimeout(function(){$this.removeClass ('open')}, 100));
            }
          });
        });
      }
      
      $(window).resize(function() {
        var windowWidth = window.innerWidth ? window.innerWidth : $(window).width();
        if(windowWidth != Drupal.TBMegaMenu.oldWindowWidth){
          Drupal.TBMegaMenu.oldWindowWidth = windowWidth;
          Drupal.TBMegaMenu.menuResponsive();
        }
      });
    },
  }
})(jQuery);

