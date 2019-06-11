(function($) {
  Drupal.behaviors.analyzedPhenotypesQualPlot = {
    attach: function (context, settings) {

    Drupal.settings.analyzedPhenotypes.forEach(function (apSettings, ctr) {
    if (apSettings.type === 'multibar') {
      // The Germplasm Name:
      ////////////////////////////////////
      var germplasmName = '';
        germplasmName = apSettings.germplasm;

      // Prepare variables.
      var experiment_id = apSettings.experiment_id,
        trait_id = apSettings.trait_id,
        method_id = apSettings.method_id,
        unit_id = apSettings.unit_id,
        xAxisLabel = apSettings.yaxis,
        yAxisLabel = "Frequency of Occurrence (percent)",
        id = apSettings.id;

      var margin = {top: 100, right: 20, bottom: 60, left: 50},
        width = 800 - margin.left - margin.right,
        height = 600 - margin.top - margin.bottom;

      var x0 = d3.scale.ordinal()
        .rangeRoundBands([0, width], .1);

      var x1 = d3.scale.ordinal();

      var y = d3.scale.linear()
        .range([height, 0]);

      var xAxis = d3.svg.axis()
        .scale(x0)
        .orient("bottom");

      var yAxis = d3.svg.axis()
        .scale(y)
        .outerTickSize(0)
        .innerTickSize(-width + margin.right)
        .orient("left");

      var color = d3.scale.ordinal()
        .range(["#BBC7BD","#0C6758","#7AB318","#253443", "#4a7dad", "#3b2d96", "#6d7f33", "#4aad9e", "#6b6b6b", "#0a8c05"]);

      // Create the canvas.
      d3.select("#"+id+" svg").remove();
      var svg = d3.select("#"+id).append("svg")
          .attr("width", width + margin.left + margin.right)
          .attr("height", height + margin.top + margin.bottom)
          .attr("id", "ap-svg-" + ctr)
        .append("g")
          .attr("transform", "translate(" + margin.left + "," + margin.top + ")");


      // Begin Bar Chart:
      d3.json('/json/phenotypes/traitplot/'+experiment_id+'/'+trait_id+'/'+method_id+'/'+unit_id, function(error, rawdata) {

        // Will contain the processed data.
        data = [];
        var dataGermplasm = new Array();

        // X-axis labels.
        var categoryNames = [],
          categoryIndex = {},
          categoryI = 0;
        // Series Labels.
        var seriesNames = [],
          seriesIndex = {},
          seriesI = 0;
        // The max number of germplasm in a category/series combo.
        var ymax = 0;
        // Keyed by the series, the max number of germplasm within that series across categories.
        var seriesMax = [];

        // For each datapoint, add to the count for the correct category/series combo.
        rawdata.forEach(function(e) {
          // If we've never seen this category then initialize it.
          if (!categoryIndex.hasOwnProperty(e.value)) {
            data[ categoryI ] = { category: e.value, values: [] };
            categoryNames.push(e.value);
            categoryIndex[ e.value ] = categoryI;
            categoryI++;
          }
          ci = categoryIndex[ e.value ];
          // If we've never seen this series then initialize it.
          if (!seriesIndex.hasOwnProperty(e.category)) {
            seriesNames.push(e.category);
            seriesIndex[ e.category ] = seriesI;
            seriesMax[ seriesI ] = 0;
            seriesI++;
          }
          si = seriesIndex[ e.category ];

          // If we haven't seen this series in the current category... initialize.
          if (typeof data[ ci ].values[ si ] === 'undefined') {
            data[ ci ].values[ si ] = { value: 0, series: e.category };
          }

          // Iterate the number of germplasm for this category/series combo.
          data[ ci ].values[ si ].value += 1;

          // Mark where germplasm was found in a count:
          //////////////////////////////////////////////////
          if (germplasmName != '' && e.germ == germplasmName) {
            dataGermplasm.push(ci + '-' + si);
          }

          // Interate the number of germplasm for this series.
          seriesMax[ si ] += 1;

        });

        // @debug console.log(seriesMax);

        // re-loop through all category/series combinations...
        categoryNames.forEach(function (c, ci) {
          seriesNames.forEach(function (s, si) {

            // Make sure all categories have all series.
            if (typeof data[ ci ].values[ si ] === 'undefined') {
              data[ ci ].values[ si ] = { value: 0, series: s };
            }

            // And change the values to a frequency (divide by the sum of all germplasm for a given series).
            data[ ci ].values[ si ].value = (data[ ci ].values[ si ].value / seriesMax[ si ]) * 100;

            // Check to see if this is the max frequency we've seen...
            if (data[ ci ].values[ si ].value > ymax) {
              ymax = data[ ci ].values[ si ].value;
            }

          });
        });

        // Sort the categories.
        categoryNames.sort(function (a,b) {
          if (a > b) { return 1; } else { return -1; }});

        // Set up the scales based on the values of the axis'.
        x0.domain(categoryNames);
        x1.domain(seriesNames).rangeRoundBands([0, x0.rangeBand()]);
        var ypadding = 0;
        if (ymax < 10) { ypadding = 1; } else { ypadding = 10; }
        y.domain([0, ymax + ypadding]);

        // X-Axis
        svg.append("g")
          .attr("class", "x axis")
          .attr("transform", "translate(0," + height + ")")
          .call(xAxis)
          .append("text")
            .attr("y", 25)
            .attr("dy", ".71em")
            .attr("x", width/2)
            .style("text-anchor", "middle")
            .style('font-weight','bold')
              .append('tspan')
                .attr('x', width/2)
                .attr('dy', "1.2em")
                .text(xAxisLabel.split(' (')[0])
              .append('tspan')
                .attr('x', width/2)
                .attr('dy', "1.2em")
                .text('('+xAxisLabel.split(' (').slice(1,5).join(' ('));

        // Y-Axis.
        svg.append("g")
          .attr("class", "y axis")
          .style("opacity", "1")
          .call(yAxis)
          .append("text")
            .attr("transform", "rotate(-90)")
            .attr("y", -45)
            .attr("dy", ".71em")
            .attr("x", -height/1.5)
            .style("text-anchor", "center")
            .style('font-weight', 'bold')
            .text(yAxisLabel);

        var slice = svg.selectAll(".slice")
          .data(data)
          .enter()
          .append("g")
            .attr("class", "g")
            .attr("transform",function(d) { return "translate(" + x0(d.category) + ",0)"; });

        slice.selectAll("rect")
          .data(function(d) { return d.values; })
          .enter()
            .append("g")
              .attr("class", "ap-rect-bar-wrapper")
              .append("rect")
                .attr("width", x1.rangeBand()-1)
                .attr("x", function(d) { return x1(d.series); })
                .style("fill", function(d) { return color(d.series) })
                .attr("y", function(d) { return y(d.value)-1; })
                .attr("height", function(d) { return height - y(d.value); })
                .on("mouseover", function(d) {
                  d3.select(this).style("fill", d3.rgb(color(d.series)).darker(2));
                })
                .on("mouseout", function(d) {
                  d3.select(this).style("fill", color(d.series));
                });

         //
         // Mark Germplasm where it is measured in each bar.
         if (germplasmName && germplasmName != '') {
           // Add div window used as the tool tip window.
           // But, first, ensure that it wasn't previously added, then no
           // need to reappend the element.
           var toolTipWindowId = 'ap-mark-germplasm-tooltip-window';
           var tipWindow = d3.select('#' + toolTipWindowId);

           var hasWindow = tipWindow.empty();
           if (hasWindow) {
             d3.select("body")
               .append("div")
                 .attr("id", toolTipWindowId)
                 // Style:
                 .style("background-color", "#72AB4D")
                 .style("cursor", "pointer")
                 .style("color", "#000000")
                 .style("font-family", "sans-serif")
                 .style("font-size", "0.7em")
                 .style("font-weight", "300")
                 .style("opacity", "0")
                 .style("padding", "5px")
                 .style("position", "absolute")
                 .style("text-align", "center")
                 .style("width", "70px");
           }

           // Add all circles:
           d3.select("#ap-svg-" + ctr).selectAll(".g").each(function(d, i) {
             d3.select(this).selectAll(".ap-rect-bar-wrapper").each(function(e, j) {
               var rectProp = {};
                 // X and Y, height of the rect.
                 rectProp.rect = d3.select(this).select("rect");
                 rectProp.height = rectProp.rect.attr("height");

                 rectProp.x = rectProp.rect.attr("x");
                 rectProp.x = parseInt(rectProp.x);

                 rectProp.y = rectProp.rect.attr("y");
                 rectProp.y = parseInt(rectProp.y);

               // Add the marker only when height has some value...
               // No value means it is there but not measured so we don't want a marker.
               if (rectProp.height && rectProp.height > 0 && dataGermplasm.includes( i + '-' + j )) {
                 var gRect = d3.select(this)
                   .append("g")
                   .on("mouseover", function() {
                     // Window on.
                     d3.select("#" + toolTipWindowId)
                       .html(germplasmName)
                       .style("left", (d3.event.pageX + 10) + "px")
                       .style("top" , (d3.event.pageY) + "px")
                       .style("opacity", "0.88");
                   })
                   .on("mouseout", function() {
                     // Window out.
                     d3.select("#" + toolTipWindowId)
                       .html("")
                       .style("left", 0)
                       .style("top" , 0)
                       .style("opacity", "0");
                   })
                   .attr("transform", "translate("+ (rectProp.x + (x1.rangeBand() / 2)) + "," + (rectProp.y - 15) +")")
                   .style("cursor", "pointer");

                 // Round handle - active handle.
                 gRect
                   .append("circle")
                     .attr("r", 5)
                     .attr("fill", "#74ac50");

                 // Pointer - point at the bar.
                 gRect
                   .append("polygon")
                     .attr("points", "0, 0 0, 12 5, 0")
                     .attr("fill", "#74ac50");
               }
             });
           });
         }
         //

         //Legend
         var legend = svg.selectAll(".legend")
           .data(data[0].values.map(function(d) { return d.series; }).reverse())
           .enter()
           .append("g")
             .attr("class", "legend")
             .attr("transform", function(d,i) { return "translate(0," + (i * 20 - margin.top) + ")"; })
             .style("opacity", "1");

         legend.append("rect")
           .attr("x", width - 18)
           .attr("width", 18)
           .attr("height", 18)
           .style("fill", function(d) { return color(d); });

         legend.append("text")
           .attr("x", width - 24)
           .attr("y", 9)
           .attr("dy", ".35em")
           .style("text-anchor", "end")
           .text(function(d) {return d; });

         if (apSettings.addWatermark) {
           if (apSettings.watermarkURL !== false) {
             tripalD3.placeWatermark({'watermark' : apSettings.watermarkURL});
           }
           else {
             tripalD3.placeWatermark();
           }
         }

      }); //end of get json.
    }});
  }}; // End of Drupal Behaviours and associated attach.
})(jQuery);
