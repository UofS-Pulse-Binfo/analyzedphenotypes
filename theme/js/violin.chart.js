(function($) {
  Drupal.behaviors.analyzedPhenotypesViolinPlot = {
    attach: function (context, settings) {

      // Draw the chart.
      var chart;
      d3.json('http://knowpulse.usask.ca/dev/tripal/json/phenotypes/violinplot/35/3626', function(error, data) {

         // Ensure the datapoint is a number.
         data.forEach(function (d) {d.value = +d.value;});

         // Draw the x/yaxis of the chart and label.
         chart = makeDistroChart({
                  data:data,
                  xName:'category',
                  yName:'value',
                  axisLabels: {xAxis: "The Number of Germplasm with a given mean per Site Year", yAxis: 'Mean Observed Values per Site Year'},
                  selector:"#tripal-ap-violin-plot",
                  margin:{top: 15, right: 60, bottom: 150, left: 75},
                  chartSize:{height:500, width:960},
                  constrainExtremes:false});
          // Render the plot.
          chart.renderViolinPlot({clamp:0});
      }); //end of get json.

  }}; // End of Drupal Behaviours and associated attach.
})(jQuery);
