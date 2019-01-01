(function($) {
  Drupal.behaviors.analyzedPhenotypesQualPlot = {
    attach: function (context, settings) {

      // Prepare variables.
      var experiment_id = Drupal.settings.analyzedPhenotypes.experiment_id,
        trait_id = Drupal.settings.analyzedPhenotypes.trait_id,
        method_id = Drupal.settings.analyzedPhenotypes.method_id,
        unit_id = Drupal.settings.analyzedPhenotypes.unit_id,
        xAxisLabel = Drupal.settings.analyzedPhenotypes.yaxis,
        yAxisLabel = "Number of Germplasm";

      var margin = {top: 100, right: 20, bottom: 50, left: 50},
        width = 960 - margin.left - margin.right,
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
        .range(["#BBC7BD","#0C6758","#7AB318","#253443", "#21597D", "#090C0E", "#D5D4E6", "#CCCCCC", "#9FA7A3"]);

      // Create the canvas.
      var svg = d3.select("#tripal-ap-violin-plot").append("svg")
          .attr("width", width + margin.left + margin.right)
          .attr("height", height + margin.top + margin.bottom)
        .append("g")
          .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

      d3.json(Drupal.settings.basePath+'/json/phenotypes/traitplot/'+experiment_id+'/'+trait_id+'/'+method_id+'/'+unit_id, function(error, rawdata) {

        // Will contain the processed data.
        data = [];
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
            seriesI++;
          }
          si = seriesIndex[ e.category ];

          // If we haven't seen this series in the current category... initialize.
          if (typeof data[ ci ].values[ si ] === 'undefined') {
            data[ ci ].values[ si ] = { value: 0, series: e.category };
          }

          // Iterate the number of germplasm for this category/series combo.
          data[ ci ].values[ si ].value += 1;

          // Check to see if this is the max num of germplasm we've seen...
          if (data[ ci ].values[ si ].value > ymax) {
            ymax = data[ ci ].values[ si ].value; 
          }
        });

        // Make sure all categories have all series.
        categoryNames.forEach(function (c, ci) {
          seriesNames.forEach(function (s, si) {
            if (typeof data[ ci ].values[ si ] === 'undefined') {
              data[ ci ].values[ si ] = { value: 0, series: s };
            }
          });
        });

        // Sort the categories.
        categoryNames.sort(function (a,b) {
          if (a > b) { return 1; } else { return -1; }});

        // Set up the scales based on the values of the axis'.
        x0.domain(categoryNames);
        x1.domain(seriesNames).rangeRoundBands([0, x0.rangeBand()]);
        var ypadding = Math.ceil(ymax / 15);
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
              .text(xAxisLabel);

        // Y-Axis.
        svg.append("g")
              .attr("class", "y axis")
              .style('opacity','1')
              .call(yAxis)
          .append("text")
              .attr("transform", "rotate(-90)")
              .attr("y", -45)
              .attr("dy", ".71em")
              .attr("x", -height/1.5)
              .style("text-anchor", "center")
              .style('font-weight','bold')
              .text(yAxisLabel);

        var slice = svg.selectAll(".slice")
              .data(data)
              .enter().append("g")
              .attr("class", "g")
              .attr("transform",function(d) { return "translate(" + x0(d.category) + ",0)"; });

        slice.selectAll("rect")
              .data(function(d) { return d.values; })
          .enter().append("rect")
              .attr("width", x1.rangeBand())
              .attr("x", function(d) { return x1(d.series); })
              .style("fill", function(d) { return color(d.series) })
              .attr("y", function(d) { return y(d.value); })
              .attr("height", function(d) { return height - y(d.value); })
              .on("mouseover", function(d) {
                d3.select(this).style("fill", d3.rgb(color(d.series)).darker(2));
              })
              .on("mouseout", function(d) {
                d3.select(this).style("fill", color(d.series));
              });

         //Legend
         var legend = svg.selectAll(".legend")
              .data(data[0].values.map(function(d) { return d.series; }).reverse())
           .enter().append("g")
              .attr("class", "legend")
              .attr("transform", function(d,i) { return "translate(0," + (i * 20 - margin.top) + ")"; })
              .style("opacity","1");

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

      }); //end of get json.

  }}; // End of Drupal Behaviours and associated attach.
})(jQuery);
