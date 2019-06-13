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
      // Image side by side, it is longer so we set text to 55% else to 70.
      var pWidth = (pos.includes('ap-left-to-right')) ? 55 : 70;
      var aspectRatio = (pos.includes('ap-left-to-right')) ? 20 : 27;

      // Float image left or right adding extra gutter so elements are
      // nicely spaced out in the container.
      var pFloat = (pos.includes('ap-float-left'))
        ? {'margin-left': '2%', 'float' : 'right'}
        : {'margin-right': '2%', 'float' : 'left'};

      if (pos.includes('ap-float-left') || pos.includes('ap-float-right')) {
        $('.ap-txt-define').css('width', pWidth + '%');
        $('.ap-txt-define').css(pFloat);

        // Render
        apImgPosition();
        // Listen to window resize and reapply render function.
        $(window).resize(apImgPosition);
      }
    }
  }

  /**
   * Function size image element.
   */
  function apImgPosition() {
    var pWidth = parentWrapper.width();
    var img = (aspectRatio / 100) * pWidth;

    imageWrapper.find('img').css('width', img + 'px');
  }
}}}(jQuery));
