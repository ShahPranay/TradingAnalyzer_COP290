document.addEventListener('DOMContentLoaded', function () {
  const ctx = document.getElementById('myChart').getContext('2d');
  var plotData = [
    { x: Date.parse('2023-08-01'), o: 221.13, h: 222.99, l: 222.13, c: 221.31 },
    { x: Date.parse('2023-08-02'), o: 221.13, h: 222.99, l: 222.13, c: 222.31 },
    { x: Date.parse('2023-08-03'), o: 221.13, h: 222.99, l: 222.13, c: 221.31 },
    { x: Date.parse('2023-08-04'), o: 221.13, h: 222.99, l: 222.13, c: 221.31 },
    { x: Date.parse('2023-08-05'), o: 221.13, h: 224.99, l: 220.13, c: 222.11 },
    { x: Date.parse('2023-08-06'), o: 221.13, h: 232.99, l: 222.13, c: 221.31 },
    { x: Date.parse('2023-08-07'), o: 221.13, h: 222.99, l: 222.13, c: 221.31 },
  ];

  var myChart = new Chart(ctx, {
    type: 'candlestick',
    data:{
      datasets: [{
        label: 'default plot',
        data: plotData
      }]
    }
  });

  // write logic to plot default graph

  function parseDates(value) {
    value.x = Date.parse(value.x);
    return value;
  }

  var update = function() {
    const stockName = document.querySelector('#stock-symbol').value;
    const period = document.querySelector('#period').value;
    const interval = document.querySelector('#interval').value;

    var data_response = fetch('http://127.0.0.1:5000/data', {
      method: 'POST',
      body: JSON.stringify({
        stock_name: stockName,
        period: period,
        interval: interval
      }),
      headers: {
        'Content-Type': 'application/json'
      }
    })
      .then(response => response.json())
      .then(data => {
        plotData = data.data.map(parseDates);
        // process plotData
        console.log(plotData);
        myChart.config.data.datasets = [
          {
            label: data.label,
            data: plotData
          }
        ]
      });
    myChart.update();
  };

  document.getElementById('update').addEventListener('click', update);
});
