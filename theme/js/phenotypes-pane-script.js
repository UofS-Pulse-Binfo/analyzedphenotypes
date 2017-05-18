(function($) {
  Drupal.behaviors.apPhenotypesPane = {
    attach: function (context, settings) {

      // CUSTOM AUTOCOMPLETE SEARCH - acs
      // Append the search form elements into.
      var acsAppendIn = 'ap-acs-form-elements-autocomplete-search';
      // Search field attributes.
      var acsAttr = {
        'id'        : 'acs-txt-search',
        // Default the field to have inactive throbber gif.
        'class'     : 'acs-search-inactive',
        'maxlength' : 50,
        // Result id attribute.
        'result'    : 'acs-container-search-result',
      };

      // Add search form elements.
      acsAppendSearchField(acsAppendIn, acsAttr);


      // # Event listener: when user types in the search field.
      // Reference the search text field.
      var acsTxtSearchField = $('#' + acsAttr.id);
      acsTxtSearchField.keyup(acsAutocompleteSearch);


      // # Function: add text field and submit button.
      function acsAppendSearchField(id, acsAttr) {
        // Container element to append search field.
        var container = $('#' + id);
        // Text field element.
        var txtField;
        txtField = 'input ';

        $.each(acsAttr, function(key, val) {
          if (key != 'result') {
            txtField += key + '=' + '"' + val + '" ';
          }
        });

        // Submit/search button.
        var btnSubmit = '<input type="button" value="" id="acs-btn-search" class="form-submit">';

        container.html('<' + txtField + '>' + btnSubmit);
      }

      // Function: auto complete search given a keyword.
      function acsAutocompleteSearch() {
        var key = acsTxtSearchField.val();
        // Show only the first 10 results.
        var limit = 10;
        // Array to hold search results.
        var items = [];

        // Search keywords.
        if (key == ' ' || key.length <= 0) {
          // Nothing to search.
          // Clear any previous search result.
          if ($('#' + acsAttr.result).length > 0) {
            $('#' + acsAttr.result).remove();
          }

          // Animation off.
          acsTxtSearchField
            .removeClass('acs-search-active')
            .addClass('acs-search-inactive');
        }
        else {
          // Something to crunch.
          var items = [];
          // TODO: Change this.
          var u = 'http://knowpulse.usask.ca/dev/reynold/analyzedphenotypes/search/trait/searchtext/' + key;

          // Start the animation.
          acsTxtSearchField
            .removeClass('acs-search-inactive')
            .addClass('acs-search-active');

          // Read get JSON.
          $.getJSON(u, function(data) {
            // Clear any previous search result.
            if ($('#' + acsAttr.result).length > 0) {
              $('#' + acsAttr.result).remove();
            }

            if (data && Object.keys(data).length > 0) {
              $.each(data, function(key, val) {
                items.push('<li id="' + key + '" title="' + val + '">' + val + '</li>');
              });

              // Add elements to DOM.
              if (items.length > 0) {
                var ul = $('#' + acsAppendIn).append('<ul id="' + acsAttr.result + '"></ul>').find('ul');
                ul.append(items.join(''));
              }

              // Add event listener.
              ul.children('li').click(function() {
                var txt = $(this).text();
                ul.remove();
                acsTxtSearchField.val(txt);
              });
            }

            // Animation off.
            acsTxtSearchField
              .removeClass('acs-search-active')
              .addClass('acs-search-inactive');
          });
        }
      }



      // INFOGRAPHICS.
      // Reference infographics container and data category container.
      var igInfoGraphics = {
        // Infographics container and data category container.
        mainContainerRef  : $('#container-infographics'),
        dataContainerRef  : $('.container-data-category'),
        // Parent container in germplasm pane and traits page.
        germContainerId   : 'ap-phenotypes-tripal-data-pane',
        traitContainerId  : 'overview-tripal-data-pane',
        // Max width the layout supports.
        maxWidth          : 700,
        // Margins.
        topMargin         : 20,
        gutter            : 40,
      };

      // Registered container used/detected.
      var igRegContainerRef = ($('#' + igInfoGraphics.germContainerId).length)
        ? $('#' + igInfoGraphics.germContainerId)
        : $('#' + igInfoGraphics.traitContainerId);

      // Get width.
      var regContainerWidth = igRegContainerRef.width();
      // Flag if pane is accessed on page load or via phenotypes link.
      var igInit;


      // # Event listeners.
      // Add event listener to window resize.
      $(window).resize(function() {
        if (igRegContainerRef.is(':visible')) {
          igGetContainerWidth();
          igRender();
        }
      });

      // When phenotypes pane link is clicked, render infographics.
      // Initialize the width to the main container of infographics.
      $('#ap-phenotypes').click(function() {
        igInit = 1;
        igRender();
      });

      // When other pane links are clicked - fixes browser
      // resets width of the container div.
      $('#tripal-chado_stock-contents-table a').not('#ap-phenotypes').click(function() {
        igInit = 0;
      });


      // # Function: get the width of parent container.
      function igGetContainerWidth() {
        regContainerWidth = igRegContainerRef.width();
      }

      // Function: position infographics.
      function igRender(w = null) {
        // Infographics data type container width, height and center.
        var containerWidth, containerHeight, containerCenter;
        // Get the parent container width.
        containerWidth = igInfoGraphics.mainContainerRef.width();

        if (w > 0) {
          containerWidth = w;
        }
        else if (igInit == 1 && containerWidth <= 100) {
          containerWidth = regContainerWidth;
        }

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


      // TITLE BAR.
      var tbTitleBar = $('#container-title-bar');
      tbTitleBar.click(function() {
        var title = $(this).find('h2').text();

        if (igInfoGraphics.mainContainerRef.is(':hidden')) {
          // When infographics is hidden
          igInfoGraphics.mainContainerRef.slideDown('fast');
          $(this).addClass('marker').html('<h2>' + title + '</h2>');

          // Re-post infographics.
          igRender();

          // Remove svg.
          $('svg').remove();

          // Reset beanplot container.
          $('#beanplot-container').css({
            'height' : 0,
            'width'  : 0,
            'margin-bottom' : 0,
          });

          // Restore trait listing.
          $('#trait-list').show();
        }
        else {
          // When infographics is shown.
          igInfoGraphics.dataContainerRef.hide();
          igInfoGraphics.mainContainerRef.slideUp('fast');

          // Minify the title bar.
          var m = $('.container-data-text em');
          var markupList = '';

          for (var i = 0; i < m.length; i++) {
            var text = m.eq(i).text();
            var info = text.split(' ');

            markupList += '<li>' + info[1] + '<em>' + info[0] + '</em></li>';
          }

          // Construct information bar.
          var d = '<div class="topic-title"><h2>' + title + '</h2></div><div class="topic-data"><ul>' + markupList + '</ul></div>';

          $(this)
            .removeClass('marker')
            .children()
            .remove();

          $(this).html(d);
        }
      });

































   $('#trait-list li').click(function() {
     $(this).parent().hide();
     $('#container-title-bar').click();

     beanplot();

     $(window).scrollTop(0,0);
   });

   function beanplot() {
      $('#beanplot-container').css({
        'height': '480px',
        'width' : '100%',
        'margin-bottom' : '30px'
      });

      // On initial load, get the initial window height and width.
      var svg, x, xAxis, y, yAxis, zAxis, height, width;
      getBPContainerDimension();

      // Chart margins and dimensions.
      // Height or width of g container of both axes.
      // Adjust to zoom in and out.
      var axesHW = 90;

      // Main chart g container.
      var bpMargin      = {};
        bpMargin.top    = 5;
        bpMargin.right  = 0;
        bpMargin.bottom = axesHW;
        bpMargin.left   = axesHW;
        bpMargin.gutter = 5;

      // Data
      var dataset = [

        {'location':'My Favourite Place', 'year':	1984, 'mean':	100, 'no':	1,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	40, 'no':	2,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	66, 'no':	4,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	67, 'no':	2,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	68, 'no':	6,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	69, 'no':	7,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	70, 'no':	5,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	71, 'no':	10, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	72, 'no':	9,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	73, 'no':	9,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	74, 'no':	15, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	75, 'no':	17, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	76, 'no':	21, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	77, 'no':	21, 'germ': 1},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	78, 'no':	29, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	79, 'no':	20, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	80, 'no':	17, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	81, 'no':	16, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	82, 'no':	22, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	83, 'no':	17, 'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	84, 'no':	9,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	85, 'no':	8,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	86, 'no':	7,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	87, 'no':	1,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	88, 'no':	3,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	89, 'no':	2,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	90, 'no':	1,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	91, 'no':	1,  'germ': 0},
        {'location':'My Favourite Place', 'year':	1984,  'mean':	92, 'no':	1,  'germ': 0},


        {'location':'Pluto', 'year':	1984, 'mean':	100, 'no':	1,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	40, 'no':	2,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	66, 'no':	4,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	67, 'no':	2,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	68, 'no':	6,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	69, 'no':	7,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	70, 'no':	5,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	71, 'no':	1, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	72, 'no':	2,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	73, 'no':	3,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	74, 'no':	5, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	75, 'no':	17, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	76, 'no':	21, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	77, 'no':	21, 'germ': 1},
        {'location':'Pluto', 'year':	1984,  'mean':	78, 'no':	29, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	79, 'no':	20, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	80, 'no':	17, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	81, 'no':	16, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	82, 'no':	22, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	83, 'no':	17, 'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	84, 'no':	9,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	85, 'no':	8,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	86, 'no':	7,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	87, 'no':	1,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	88, 'no':	3,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	89, 'no':	2,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	90, 'no':	1,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	91, 'no':	1,  'germ': 0},
        {'location':'Pluto', 'year':	1984,  'mean':	92, 'no':	1,  'germ': 0},

        {'location':'Mars', 'year':	1986,  'mean':	100, 'no':	1,  'germ': 0},
        {'location':'Mars', 'year':	1986,  'mean':	40, 'no':	1,  'germ': 0},
        {'location':'Mars', 'year':	1986,  'mean':	66, 'no':	2,  'germ': 0},
        {'location':'Mars', 'year':	1986,  'mean':	67, 'no':	2,  'germ': 0},
        {'location':'Mars', 'year':	1986,  'mean':	68, 'no':	3,  'germ': 0},
        {'location':'Mars', 'year':	1986,  'mean':	69, 'no':	1,  'germ': 0},
        {'location':'Mars', 'year':	1986,  'mean':	70, 'no':	1,  'germ': 0},
        ];

      var location_year = d3.nest()
        .key(function(d) { return d.year + ':' + d.location; })
        .sortKeys(d3.ascending)
        .entries(dataset);

      var lyCount = location_year.length,
          maxMean = d3.max(dataset.map(function(d) { return d.mean; })),
          minMean = d3.min(dataset.map(function(d) { return d.mean; })),
          maxNo   = d3.max(dataset.map(function(d) { return d.no; }));



      // Add chart elements.
      // Canvas.
      svg = d3.select('#beanplot-container')
        .append('svg')
          .attr('id', 'beanplot-svg-container');

      // Define Gradient.
      var bpGradient = svg.append('defs')
        .append('linearGradient')
        .attr('id', 'bp-gradient')
        .attr('x1', '0%')
        .attr('x2', '100%')
        .attr('x2', '0%')
        .attr('y2', '100%')
        .attr('spreadMethod', 'pad');

      bpGradient.append('stop')
        .attr('offset', '0%')
        .attr('stop-color', '#EAEAEA')
        .attr('stop-opacity', 1);

      bpGradient.append('stop')
        .attr('offset', '100%')
        .attr('stop-color', '#C3C3C3')
        .attr('stop-opacity', 1);

      var bpGradientGerm = svg.append('defs')
        .append('linearGradient')
        .attr('id', 'bp-gradient-germ')
        .attr('x1', '0%')
        .attr('x2', '80%')
        .attr('x2', '0%')
        .attr('y2', '100%')
        .attr('spreadMethod', 'pad');

      bpGradientGerm.append('stop')
        .attr('offset', '0%')
        .attr('stop-color', '#AFD0A1')
        .attr('stop-opacity', 1);

      bpGradientGerm.append('stop')
        .attr('offset', '100%')
        .attr('stop-color', '#9AC585')
        .attr('stop-opacity', 1);


      // Tool tip.
      var infoBox = d3.select('body')
        .append('div')
        .attr('class', 'tool-tip')
        .style('opacity', 0);


      // Y
      var yAxisWrapper = svg.append('g')
        .attr('id', 'bp-g-yaxiswrapper')
        .attr('class', 'bp-g-axis')


      // Label.
      var yLabel = 'Mean Observed Values per Site Year';
      yAxisWrapper.append('g')
        .attr('id', 'bp-y-text-label')
        .append('text')
          .attr('id', 'bp-y-axis-label')
          .text(yLabel);


      // Scale.
      // Add some value to min and max value, that will serve as
      // margin/padding for bean plot.

      var r = maxMean;
      var newMaxMean = top(r);
      // Initial width of the chart area height.
      var range = height - bpMargin.top - bpMargin.bottom - bpMargin.gutter

      var k = minMean;
      var newMinMean = bottom(k);
      y = d3.scale.linear()
        .domain([newMinMean, newMaxMean])
        .range([range, 0])
        .nice();

      var ticks     = y.ticks(),
          ticksMax  = d3.max(ticks),
          ticksPair = d3.pairs(ticks),
          ticksDiff = ticksPair[0][1] - ticksPair[0][0],
          ticksAll  = []

      for(var f = newMinMean; f <= ticksMax; f++) {
        // When difference is 1, use .5;
        if (ticksDiff <= 1) {
          var e = f + 0.5;

          ticksAll.push(e);
        }

        ticksAll.push(f);
      }

      ticksNo   = ticksAll.length;

      // Sort tick values.
      ticksAll.sort(function(a, b) {
        return a-b;
      });


      if (ticksAll[0] > 0) {
       var breakLineHeight = 29;

       // Break line when y axis scale does not strat at 0.
        var breakLine = yAxisWrapper.append('g')
          .attr('id', 'break-line')
          .append('polyline')
          .attr('fill', 'none')
          .attr('stroke', '#000000')
          .attr('shape-rendering', 'crispEdges');

        // Break line at 0.
        var isZero =  yAxisWrapper
            .append('g')
            .attr('class', 'is-zero')
              .append('text')
              .text('0');
      }


      yAxis = d3.svg.axis()
        .scale(y)
        .orient('left')
        .ticks(ticksNo)
        .tickValues(ticksAll)
        .tickFormat(d3.format('d'));

      var yMeanValues = yAxisWrapper.append('g')
        .attr('id', 'bp-y-scale')
        .attr('class', 'bp-axis')
        .call(yAxis);

      d3.selectAll('#bp-y-scale text')
        .attr('x', -15)
        .text(function(d) {
          return (d%ticksDiff == 0) ? d : null;
        });

      d3.selectAll('#bp-y-scale line')
        .attr('x2', function(d) {
          return (d%ticksDiff == 0) ? -12 : -6;
        });


      // X
      var xAxisWrapper = svg.append('g')
        .attr('id', 'bp-g-xaxiswrapper')
        .attr('class', 'bp-g-axis');

      // Label.
      var xLabel = 'The Distribution of Mean Days till 10% of Plants have 1/2 Pods Mature (R7; days) per Site Year';
      xAxisWrapper.append('g')
        .attr('id', 'bp-x-text-label')
        .append('text')
          .attr('class', 'text-axes-label')
          .text(xLabel);

      x = d3.scale.ordinal()
        .domain(location_year.map(function(d) {
          return d.key;
        }));

      xAxis = d3.svg.axis()
        .orient('bottom');

      var xLocationYear = xAxisWrapper.append('g')
        .attr('id', 'bp-x-scale')
        .attr('class', 'bp-axis');


      // Z
      var z = d3.scale.linear();


      // Beanplot elements.
      // g container where all chart elements will be rendered.
      // Adjust beanplotMargin to change coordinate.
      var chartWrapper = svg.append('g')
       .attr('id', 'bp-g-chartwrapper');

      // Create g container for each bean/plot.
      var bpWrapper = chartWrapper
       .selectAll('g')
       .data(location_year)
       .enter()
       .append('g')
         .attr('id', function(d, i) {
           return 'bp-location-year-' + i;
         })
         .attr('class', 'bp-g-location-year');

      var rect = bpWrapper.selectAll('rect')
       .data(function(d) {
         return d.values;
       })
       .enter()
       .append('g')
         .on('mousemove', function(d) {
           d3.select(this).style('opacity', 0.5);

           infoBox.transition().style('opacity', 1);
           infoBox
             .html('Mean: ' + d.mean + ' (' + d.no + ' Germplasms)')
             .style('left', (d3.event.pageX + 10) + 'px')
             .style('top', (d3.event.pageY) + 'px');
         })
         .on('mouseout', function(d) {
           d3.select(this).style('opacity', 1);
           infoBox.transition().style('opacity', 0);
         })
       .append('rect')
         .attr('fill', function(d) {
           return (d.germ == 1) ? 'url(#bp-gradient-germ)' : 'url(#bp-gradient)';
         })
         .attr('stroke', function(d) {
           return (d.germ == 1) ? '#72AB4D' : '#A5A5A5';
         });

      // Render the chart elements.
      renderBP();


      // Position the chart elements to the right coordinates based
      // height and width returned.
      function renderBP() {
        // Get the new dimension.
        getBPContainerDimension();

        var bp = {};
          bp.chartAreaWidth = width - bpMargin.left - bpMargin.right - bpMargin.gutter;
          bp.chartAreaHeight = height - bpMargin.top - bpMargin.bottom - bpMargin.gutter;
          bp.chartEachly = Math.round(bp.chartAreaWidth/lyCount);
          bp.chartBarHeight = bp.chartAreaHeight/ticksAll.length;


        // Apply the new dimension to chart elements.

        // Canvas.
        svg
          .attr('height', height)
          .attr('width', width);

        // Y
        yAxisWrapper
          .attr('transform', 'translate(0, ' + bpMargin.top + ')');

        // label - 20px text height of the y label.
        d3.select('#bp-y-text-label text')
          .attr('transform', function() {
            return 'translate(' + (bpMargin.gutter + 20) + ',' + Math.round((height + bpMargin.bottom)/2) +') rotate(-90)';
          });

        // Scale - Mean values.
        yMeanValues
          .attr('transform', 'translate(' + bpMargin.left + ',' + bpMargin.top + ')');

        // Add the break line.
        if (breakLine && isZero) {
          breakLineHeight = 29;
          breakLine
            .attr('points', function() {
              var cLeft = bpMargin.left;
              var cBottom = bp.chartAreaHeight + (bpMargin.gutter * 2) + 4;

              return cLeft + ',' + (cBottom - 10) + ' ' + cLeft + ',' + cBottom + ' ' + (cLeft + 10) + ',' + cBottom + ' ' + (cLeft - 10) + ',' + (cBottom + 10) + ' ' + cLeft + ',' + (cBottom + 10) + ' ' + cLeft + ',' + (cBottom + 20) + ' ' + (cLeft - 10) + ',' + (cBottom + 20);
            });

          isZero
            .attr('transform', function() {
              return 'translate('+ (bpMargin.left - 20) + ',' + (bp.chartAreaHeight + 38) +')';
            });
        }
        else {
          breakLineHeight = 0;
        }


        // X
        // label - 20px text height of the y label.
        d3.select('#bp-x-text-label')
          .attr('text-anchor', 'middle')
          .attr('transform', function() {
            return 'translate(' + (Math.round(width/2) - bpMargin.left) + ',' + (bpMargin.bottom - bpMargin.gutter) + ')';
          });

        // Scale Location and Year.
        xAxisWrapper
          .attr('transform', 'translate(' + bpMargin.left + ',' + (height - bpMargin.bottom + breakLineHeight) + ')');

        x
          .rangeRoundBands([0, bp.chartAreaWidth]);
          xAxis.scale(x);

        xLocationYear
          .call(xAxis)
        .selectAll('#bp-x-scale text')
        .call(wrapWords, x.rangeBand());


        // Z
        z
          .domain([0, maxNo])
          .range([0, bp.chartEachly - bpMargin.gutter - 60]);


        // Beanplot chart.
        chartWrapper
          .attr('height', bp.chartAreaHeight)
          .attr('width', bp.chartAreaWidth)
          .attr('transform', 'translate(' + (bpMargin.left + bpMargin.gutter) + ',' + bpMargin.top + ')');

        d3.selectAll('.bp-g-location-year')
          .attr('width', bp.chartEachly - 60)
          .attr('transform', function(d, i) {
            var x = bp.chartEachly * i;

            return 'translate(' + x + ',' + bpMargin.top + ')';
          });

        rect
          .attr('height', bp.chartBarHeight)
          .style('opacity', 0)
          .attr('width', function(d) {
            return Math.round(z(d.no));
          })
          .attr('x', function(d) {
            var w = d3.select(this).attr('width');
                w = parseInt(w);

            var wHalf = w/2,
                ww = bp.chartEachly,
                wwHalf = ww/2;

             return (wwHalf - wHalf) - bpMargin.gutter;
          })
          .attr('y', function(d) {
            return posBar(d.mean, bp.chartBarHeight);
          })
          .transition()
          .duration(function() {
            return randomNumber();
          })
          .ease('back')
          .style('opacity', 1);
      }



      // HELPER FUNCTIONS:

      // Position the bars into the right y (mean) scale.
      function posBar(mean, h) {
        var index, r;

        //mean = (mean < 0) ? Math.floor(mean) : Math.ceil(mean);
        r = (mean % 1);

        // Find the ticks that corresponds to the mean value.
        // Then get the y coordinate value and use that to plot bar.
        ticksAll.forEach(function(d, i) {
          if (d == mean) {
            index = i;
          }
        });

        var s;
        var g = d3.select('#bp-y-scale').selectAll('.tick')
          .filter(function(d, i) {
            if (i == index) {
              s = d3.select(this).attr('transform');
            }
          });

        var t = d3.transform(s),
        y = Math.round(t.translate[1]);

        return y - Math.round(h/2);
      }

      // Debugging function. Echo the contents of d.
      function echo(d) {
        alert(JSON.stringify(d));
        //console.log(JSON.stringify(d));
        //console.log(d);
      }

      // Get the height and width of the chart container
      // and use it to estimate the amount of area chartable.
      function getBPContainerDimension() {
        // Reference chart container.
        var container = d3.select('#beanplot-container');

        // Get height and width.
        var containerHeight = container.style('height');
        var containerWidth = container.style('width');

        // Set the the global var accordingly.
        height = parseInt(containerHeight, 10);
        width = parseInt(containerWidth, 10);
      }
      //

      // Function stack text on top of each other.
      // e.g.   2016    (year)
      //      Saskatoon (location)
      function wrapWords(text, width) {
        text.each(function() {
          // Reference text.
          var text  = d3.select(this);
          // Read the words in the text.
          var words = text.text().split(/:/);
          // Clear the text so no duplicate label shown.
          text.text(null);

          var word,
              line = [],
              lineNumber = 0,
              lineHeight = 1 // ems
              y = text.attr('y') - bpMargin.gutter,
              dy = parseFloat(text.attr('dy'));

          while (word = words.pop()) {
            text.append('tspan')
              .attr('class', 'bp-tspan')
              .attr('x', 0)
              .attr('y', y)
              .attr('dy', ++lineNumber * lineHeight + dy + 'em')
              .text(word);
          }
        });
      }

      // Function increase the max mean value.
      function top(v) {
        var newMean;

        // Exclude decimal values.
        v = (v > 0) ? Math.floor(v) : Math.ceil(v);

        if (v < 5) {
          newMean = 5;
        }
        else {
          // 5 and above.
          var mod = (v % 10);
          var no = v + mod;

          newMean = no + 5;
        }

        return newMean;
      }

      // Function decrease the min value.
      function bottom(v) {
        var newMean;

        // Exclude decimal values.
        v = (v >= 0) ? Math.floor(v) : Math.ceil(v);

        if (v < 10) {
          newMean = 0;
        }
        else {
         // 10 and above
         var mod = (v % 10);
         var no = v + mod;

         newMean = no - 10;
        }

        return newMean;
      }

      // Generate random numbers.
      function randomNumber() {
        var min = 1;
        var max = 10;

        return (Math.random() * (max - min) + min ) * 100;
      }
    }




  }};
}(jQuery));




