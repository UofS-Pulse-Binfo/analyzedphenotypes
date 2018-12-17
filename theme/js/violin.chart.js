(function($) {
  Drupal.behaviors.analyzedPhenotypesViolinPlot = {
    attach: function (context, settings) {

      // Draw the chart.
      var chart;
      var experiment_id = Drupal.settings.analyzedPhenotypes.experiment_id;
      var trait_id = Drupal.settings.analyzedPhenotypes.trait_id;
      var method_id = Drupal.settings.analyzedPhenotypes.method_id;
      var unit_id = Drupal.settings.analyzedPhenotypes.unit_id;

      d3.json('http://knowpulse.usask.ca/dev/tripal/json/phenotypes/traitplot/'+experiment_id+'/'+trait_id+'/'+method_id+'/'+unit_id, function(error, data) {

          d3.selectAll('#tripal-ap-violin-plot .inner-wrapper').remove();

         // Ensure the datapoint is a number.
         data.forEach(function (d) {d.value = +d.value;});

         // Draw the x/yaxis of the chart and label.
         chart = makeDistroChart({
                  data:data,
                  xName:'category',
                  yName:'value',
                  axisLabels: {
                    xAxis: Drupal.settings.analyzedPhenotypes.xaxis, 
                    yAxis: Drupal.settings.analyzedPhenotypes.yaxis
                  },
                  selector:"#tripal-ap-violin-plot",
                  margin:{top: 15, right: 60, bottom: 75, left: 75},
                  chartSize:{height:500, width:960},
                  constrainExtremes:false});

          // Render the plot.
          chart.renderBoxPlot();
          chart.renderViolinPlot({
            clamp:0, 
            colors:['#314355']
          });
          chart.boxPlots.show({
            reset:true, 
            showWhiskers:true,
            showOutliers:false,
            boxWidth:10,
            lineWidth:15,
            colors:['#555']
          });


      }); //end of get json.

  }}; // End of Drupal Behaviours and associated attach.
})(jQuery);
