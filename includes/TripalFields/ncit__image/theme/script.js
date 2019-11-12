/**
 * @file
 * Manage phenotype image stacking and positioning.
 */

(function ($) {
Drupal.behaviors.PhenotypeImage = {
  attach: function(context, settings) {
  if ($('.ap-phenotype-image')) {
    var imageWrapper = $('#ap-field-phenotype-image-wrapper');
    var parentWrapper = imageWrapper.parents().closest('.ap-phenotype-image');

    parentWrapper.find('p')
      .addClass('ap-txt-define');

    var pos = imageWrapper.attr('class');
    if (pos) {
      if (pos.includes('ap-float-left') || pos.includes('ap-float-right')) {
        // P.
        var pWidth   = (pos.includes('ap-left-to-right')) ? 55 : 70;
        var pFloat   = (pos.includes('ap-float-right'))
          ? {'margin-right': '1%', 'float' : 'left'}
          : {'margin-left' : '1%', 'float' : 'right'};

        $('.ap-txt-define').css('width', pWidth + '%');
        $('.ap-txt-define').css(pFloat);

        // IMG.
        apImgPosition();
        $(window).resize(apImgPosition);
      }
    }
  }


  /**
   * Function size image element.
   */
  function apImgPosition() {
    var w = (pos.includes('ap-left-to-right')) ? 40 : 25;
    var imgW = (w / 100) * parentWrapper.width();

    imageWrapper.find('img').css('width', function() {
      var w;

      if (imageWrapper.find('img').length < 2) {
        w = imgW;
      }
      else {
        w = (pos.includes('ap-top-to-bottom')) ? imgW : ((imgW / 2) - 5);
      }

      return w + 'px';
    });
  }
}}}(jQuery));
