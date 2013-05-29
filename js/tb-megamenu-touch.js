Drupal.TBMegaMenu = Drupal.TBMegaMenu || {};

(function ($) {
    Drupal.TBMegaMenu.isTouch = 'ontouchstart' in window && !(/hp-tablet/gi).test(navigator.appVersion);
    if(Drupal.TBMegaMenu.isTouch){
        $.fn.touchmenu = function(){

            if(!$(document).data('touchmenu')){
                $(document).data('touchmenu', 1).data('touchitems', $()).on('click hidesub', function(){
                    $(document).removeClass('hoverable')
                        .data('touchitems').data('noclick', 0).removeClass('open');
                });

                if (navigator.userAgent.match(/(iPad|iPhone);.*CPU.*OS 6_\d/i)){ 
                    $(document.body).children(':not(.nav)').on('click', function(){
                        $(document).trigger('hidesub');
                    });
                }
            }

            return this.each(function(){	
                var itemsel = $(this).has('.mega').length ? 'li.mega' : 'li.parent',
                    tbitems = $(this).find(itemsel),
                    reset = function(){
                            $(this).data('noclick', 0);
                    },
                    onTouch = function(e){
                        e.stopPropagation();

                        $(document.body).addClass('hoverable');

                        var tbitem = $(this),
                            val = !tbitem.data('noclick');

                        if(val){
                            var jchild = tbitem.children('.dropdown-menu'),
                                hasopen = tbitem.hasClass('open'),
                                style = jchild.prop('style'),
                                display = style ? style['display'] : '';

                            if(jchild.css('display', 'none').css('display') == 'none'){ //normal or hide when collapse
                                jchild.css('display', display);

                                //at initial state, test if it is display: none !important, 
                                //if true, we will open this link (val = 0)
                                if(!hasopen){	
                                    //add open class, 
                                    //iphone seem have buggy when we modify display property
                                    //it does not trigger hover CSS
                                    $(document).data('touchitems').removeClass('open');
                                    tbitem.addClass('open').parentsUntil('.nav').filter(itemsel).addClass('open');

                                    val = jchild.css('display') != 'none';
                                }

                            } else { //always show
                                val = 0;
                            }

                            jchild.css('display', display);
                        }

                        // reset all
                        tbitems.data('noclick', 0);
                        tbitem.data('noclick', val);

                        if(val){
                            $(this) //reset, sometime the mouseenter does not refire, so we reset to enable click
                                .data('rsid', setTimeout($.proxy(reset, this), 500))
                                .parent().parentsUntil('.nav').filter(itemsel).addClass('open');							
                        }
                    },
                    onClick = function(e){
                        e.stopPropagation();

                        if($(this).data('noclick')){
                            e.preventDefault();
                            tbitems.removeClass('open');
                            $(this).addClass('open').parentsUntil('.nav').filter(itemsel).addClass('open');
                        } else {
                            var href = $(this).children('a').attr('href');
                            if(href){
                                window.location.href = href;
                            }
                        }
                    };

                tbitems.on('mouseenter', onTouch).data('noclick', 0);
                $(this).find('li').on('click', onClick);

                $(document).data('touchitems', $(document).data('touchitems').add(tbitems));
            });
        };
    }
        
  Drupal.behaviors.tbMegaMenuTouchAction = {
    attach: function(context) {
	$('html').addClass(Drupal.TBMegaMenu.isTouch ? 'touch' : 'no-touch');      
        if(Drupal.TBMegaMenu.isTouch){
            $('ul.nav').has('.dropdown-menu').touchmenu();
        }
    }
  }
})(jQuery);

