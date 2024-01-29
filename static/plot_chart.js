document.addEventListener('DOMContentLoaded', function () {
  const ctx = document.getElementById('myChart').getContext('2d');
  var myChart = new Chart(ctx, {
    type: 'candlestick'
  });

  // write logic to plot default graph

  var update = function() {
    const stockName = document.querySelector('#stock-symbol').value;
    const period = document.querySelector('#period').value;
    const interval = document.querySelector('#interval').value;

    fetch('/data', {
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
        console.log("here");
        console.log(data);
        myChart.config.data = data;
      });
    myChart.update();
  };

  document.getElementById('update').addEventListener('click', update);
});
