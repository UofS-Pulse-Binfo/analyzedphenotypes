/**
 * @file
 * Contains behavior of analyzed phenotypes trait summary fields.
 */
(function($) {
  Drupal.behaviors.apTraitSummaryFieldgroup = {
    attach: function (context, settings) {
/*
      // Reference elements.
      var apInfoGraphics = {
        // Main wrapper of all field items/elements.
        mainContainer : $('#ap-trait-summary-field-wrapper'),
        // Main container for every item/elements.
        dataContainer : $('.container-data-category'),

        // Dimensions, width and gutters:
        topMargin : 20,  // Bottom of infographics to the top of the next (vertically).
        maxWidth  : 700, // Minimum width the infographics can support.
        gutter    : 40,  // Right of infographics to the left of the next (horizontally).
      };


      // Placement of items is based on the width of the wrapper/container.
      apRender();

      // Listen to window resize and render with the new window size.
      $(window).resize(function() {
        if (apInfoGraphics.mainContainer.is(':visible')) {
          apRender();
        }
      });



      function apRender() {
        var container = {};

        container.width = apInfoGraphics.mainContainer.width();
        // Calculate the center which will be for center item (ie: 5 items / 2).
        container.center = Math.round(container.width / 2);

        // Every items.
        var items = {};
          items.container = apInfoGraphics.dataContainer;
          items.count     = items.container.length;
          items.height    = items.container.height();
          items.width     = items.container.width();
          items.center    = Math.round(items.width / 2);

        // To begin, all items are hidden via css rule display.
        // But in case they are shown via previous call, hide them once again.
        items.container.hide();

        var xPos, yPos, levels, add, extra, m, s, e, t, h, v;

        // Compute levels, first, second etc. rows horizontally.
        levels = Math.floor(items.count / 2);

        if (items.count % 2 == 0) {
          // Four or divisible by 2 items, placement is same on either sides.
          levels = levels;
          extra = false;
        }
        else {
          // Odd count, place the remainder of dividing by two in the center.
          levels += 1;
          extra = true;
        }

        if (extra) {
          // If we have extra one (odd count) the extra would be of the same
          // level as the highest level. Account for gutters too.
          h = levels - 1;
          v = apInfoGraphics.gutter;
        }
        else {
          h = levels;
          v = (apInfoGraphics.topMargin * 3);
        }

        // Less horizontal space, use vertical space instead.
        container.height = (items.height + (items.height / 2) * h) + v;
        if (container.width <= apInfoGraphics.maxWidth && extra) {
          container.height += items.height;
        }
        alert(container.height);
        apInfoGraphics.mainContainer.css('height', container.height);

        // Loop to each levels placing each item where they belong.
        for (var i = 0; i < levels; i++) {
          // establish start and end of inner loop
          s = (i == 0) ? 0 : (e + 1);
          add = (extra && i == 0) ? 2 : 1;
          e = s + add;

          // Loop on each data see if a data belong to this level.
          for (var j = s; j <= e; j++) {
            m = (i == 0) ? apInfoGraphics.topMargin : items.center;

            // X POSITION:
            if (j % 2 == 0) {
              xPos = (container.center - m) - items.width - apInfoGraphics.gutter;

              if (container.width <= apInfoGraphics.maxWidth) {
                xPos = container.center - items.width - apInfoGraphics.gutter;
              }
            }
            else {
              if (j == 1) {
                xPos = (container.center + m) + apInfoGraphics.gutter;
              }
              else {
                xPos = (container.center - m) + apInfoGraphics.gutter + items.width;
              }

              if (container.width <= apInfoGraphics.maxWidth) {
                xPos = container.center + apInfoGraphics.gutter;
              }
            }


            // Y POSITION:
            yPos = (i == 0)
              ? apInfoGraphics.topMargin
              : ((items.height + apInfoGraphics.gutter) * i) + apInfoGraphics.topMargin;

            if (container.width <= apInfoGraphics.maxWidth && extra) {
              yPos += items.height;
            }

            if (extra && i == 0) {
              if (j == 0 && container.width > apInfoGraphics.maxWidth) {
                xPos = (container.center - items.center) - items.width - apInfoGraphics.gutter;
              }
              else if (j == 1) {
                xPos = container.center - items.center;

                yPos = (container.width <= apInfoGraphics.maxWidth)
                  ? yPos - items.height - apInfoGraphics.topMargin
                  : yPos - apInfoGraphics.topMargin;
              }
              else {
                xPos = (container.width < apInfoGraphics.maxWidth)
                  ? container.center + apInfoGraphics.gutter
                  : (container.center - items.center) + items.width + apInfoGraphics.gutter;
              }
            }


            // REDER X AND Y:
            items.container.eq(j).delay(j * 100)
              .css({left: xPos, top: yPos})
              .fadeIn('slow');

          // Inner loop j.
          }
        // Main loop i.
        }
      }
*/





      // INFOGRAPHICS.
      // Reference infographics container and data category container.
      var igInfoGraphics = {
        // Infographics container and data category container.
        mainContainerRef  : $('#ap-trait-summary-field-wrapper'),
        dataContainerRef  : $('.container-data-category'),
        // Parent container in germplasm pane and traits page.
        germContainerId   : 'ap-trait-summary-field-wrapper',
        traitContainerId  : 'ap-trait-summary-field-wrapper',
        // Max width the layout supports.
        maxWidth          : 700,
        // Margins.
        topMargin         : 50,
        gutter            : 40,
      };




      // Registered container used/detected.
     // Registered container used/detected.
      var igRegContainerRef = igInfoGraphics.mainContainerRef;
      // Get width.
      var regContainerWidth = igRegContainerRef.width();
      // Flag if pane is accessed on page load or via phenotypes link.
      var igInit;

      igRender();

      // Function: position infographics.
      function igRender(w = null) {
        // Infographics data type container width, height and center.
        var containerWidth, containerHeight, containerCenter;
        // Get the parent container width.
        containerWidth = igInfoGraphics.mainContainerRef.width();



        // Half width of the main container of the data units
        // which will be the center anchor when rendering.
        containerCenter = Math.round(containerWidth / 2);

        // Infographics Data Categories.
        var igType = {};
          igType.ref    = igInfoGraphics.dataContainerRef;
          igType.count  = igType.ref.length;
          igType.height = igType.ref.height();
          igType.width  = igType.ref.width();
          igType.center = Math.round(igType.width / 2);

        // Start by hiding all infographics data types.
        igType.ref.hide();

        var xPos, yPos, levels, add, extra, m, s, e, t, h, v;

        levels = Math.floor(igType.count / 2);
        if (igType.count % 2 == 0) {
          levels = levels;
          extra = false;
        }
        else {
          levels += 1;
          extra = true;
        }

        if (extra) {
          h = levels - 1;
          v = igInfoGraphics.gutter;
        }
        else {
          h = levels;
          v = (igInfoGraphics.topMargin * 3);
        }

        containerHeight = ((igType.height + (igType.height / 2)) * h) + v;

        if (containerWidth <= igInfoGraphics.maxWidth && extra) {
          containerHeight += igType.height;
        }

        igInfoGraphics.mainContainerRef.css('height', containerHeight);


        // Loop: levels
        for (var i = 0; i < levels; i++) {
          // establish start and end of inner loop
          s = (i == 0) ? 0 : (e + 1);
          add = (extra && i == 0) ? 2 : 1;
          e = s + add;

          // Loop: data
          for (var j = s; j <= e; j++) {
            m = (i == 0) ? igInfoGraphics.topMargin : igType.center;

            // X coordinate.
            if (j % 2 == 0) {
              xPos = (containerCenter - m) - igType.width - igInfoGraphics.gutter;

              if (containerWidth <= igInfoGraphics.maxWidth) {
                xPos = containerCenter - igType.width - igInfoGraphics.gutter;
              }
            }
            else {
              if (j == 1) {
                xPos = (containerCenter + m) + igInfoGraphics.gutter;
              }
              else {
                xPos = (containerCenter - m) + igType.width + igInfoGraphics.gutter;
              }

              if (containerWidth <= igInfoGraphics.maxWidth) {
                xPos = containerCenter + igInfoGraphics.gutter;
              }
            }


            // Y coordinate.
            if (i == 0) {
              yPos = igInfoGraphics.topMargin;
            }
            else {
              yPos = ((igType.height + igInfoGraphics.gutter) * i) + igInfoGraphics.topMargin;
            }

            if (containerWidth <= igInfoGraphics.maxWidth && extra) {
              yPos += igType.height;
            }


            // Extra 1 element in the first level.
            // Override x and y coordinates.
            if (extra && i == 0) {
              // # # # - create 3 elements in the first level.
              // # # - otherwise, 2 items when elements can be paired.
              if (j == 0) {
                if (containerWidth > igInfoGraphics.maxWidth) {
                  xPos = (containerCenter - igType.center) - igType.width - igInfoGraphics.gutter;
                }
              }
              else if (j == 1) {
                xPos = containerCenter - igType.center;

                if (containerWidth <= igInfoGraphics.maxWidth) {
                  yPos = yPos - igType.height - igInfoGraphics.topMargin;
                }
                else {
                  yPos = yPos - igInfoGraphics.topMargin;
                }
              }
              else {
                if (containerWidth <= igInfoGraphics.maxWidth) {
                  xPos = containerCenter + igInfoGraphics.gutter;
                }
                else {
                  xPos = (containerCenter - igType.center) + igType.width + igInfoGraphics.gutter;
                }
              }
            }


            // Render.
            igType.ref.eq(j).delay(j * 100)
            .css({
              left : xPos,
              top  : yPos,
            })
            .fadeIn('slow');
          }
        }
      }






























  }};
}(jQuery));




