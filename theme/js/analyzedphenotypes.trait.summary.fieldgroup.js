/**
 * @file
 * Contains behavior of analyzed phenotypes trait summary fields.
 */
(function($) {
  Drupal.behaviors.apTraitSummaryFieldgroup = {
    attach: function (context, settings) {
      // Reference elements.
      var apInfographics = {
        // Main wrapper of all field items/elements.
        mainContainer : $('#ap-trait-summary-field-wrapper'),
        // Main container for every item/elements.
        dataContainer : $('.container-data-category'),

        // Dimensions, width and gutters:
        topMargin : 20,  // Bottom of infographics to the top of the next (vertically).
        maxWidth  : 700, // Minimum width the infographics can support.
        gutter    : 40,  // Right of infographics to the left of the next (horizontally).
      };


      // Place items.
      apRender();

      // Listen to window resize.
      $(window).resize(function() {
        if (apInfographics.mainContainer.is(':visible')) {
          apRender();
        }
      });


      // Function render items.
      function apRender() {
        // Item container profile - height, width, center point...
        var container = {};
        container.width  = apInfographics.mainContainer.width();
        container.center = Math.round(container.width / 2);

        var items = {};
          items.container = apInfographics.dataContainer;
          items.count     = items.container.length;
          items.height    = items.container.height();
          items.width     = items.container.width();
          items.center    = Math.round(items.width / 2);

        // Start with a hidden set of items to smoothen animations.
        items.container.hide();

        // Calculate levels of items - 1st row, 2nd row ...
        var xPos, yPos, levels, add, extra, m, s, e, t, h, v;
        levels = Math.floor(items.count / 2);

        if (items.count % 2 == 0) {
          levels = levels;
          extra = false;
        }
        else {
          levels += 1;
          extra = true;
        }

        if (extra) {
          h = levels - 1;
          v = apInfographics.gutter;
        }
        else {
          h = levels;
          v = (apInfographics.topMargin * 3);
        }

        container.height = ((items.height + (items.height / 2)) * h) + v;
        if (container.width <= apInfographics.maxWidth && extra) {
          container.height += items.height;
        }

        apInfographics.mainContainer.css('height', container.height);

        // Begin placement.
        for (var i = 0; i < levels; i++) {
          s = (i == 0) ? 0 : (e + 1);
          add = (extra && i == 0) ? 2 : 1;
          e = s + add;

          // Loop: data
          for (var j = s; j <= e; j++) {
            m = (i == 0) ? apInfographics.topMargin : items.center;

            // X:
            if (j % 2 == 0) {
              xPos = (container.center - m) - items.width - apInfographics.gutter;

              if (container.width <= apInfographics.maxWidth) {
                xPos = container.center - items.width - apInfographics.gutter;
              }
            }
            else {
              xPos = (j == 1)
                ? (container.center + m) + apInfographics.gutter
                : (container.center - m) + items.width + apInfographics.gutter;

              if (container.width <= apInfographics.maxWidth) {
                xPos = container.center + apInfographics.gutter;
              }
            }


            // Y:
            yPos = (i == 0)
              ? apInfographics.topMargin
              : ((items.height + apInfographics.gutter) * i) + apInfographics.topMargin;

            if (container.width <= apInfographics.maxWidth && extra) {
              yPos += items.height;
            }

            if (extra && i == 0) {
              if (j == 0) {
                if (container.width > apInfographics.maxWidth) {
                  xPos = (container.center - items.center) - items.width - apInfographics.gutter;
                }
              }
              else if (j == 1) {
                xPos = container.center - items.center;
                yPos = (container.width <= apInfographics.maxWidth)
                  ? yPos - items.height - apInfographics.topMargin
                  : yPos - apInfographics.topMargin;

              }
              else {
                xPos = (container.width <= apInfographics.maxWidth)
                  ? container.center + apInfographics.gutter
                  : xPos = (container.center - items.center) + items.width + apInfographics.gutter;
              }
            }


            // PLACE ITEM IN X AND Y:
            items.container.eq(j).delay(j * 100)
              .css({left: xPos, top: yPos})
              .fadeIn('slow');
          }
        }
      }
}}}(jQuery));
