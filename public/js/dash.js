option = {
    backgroundColor: '#1b1b1b',
    tooltip : {
        formatter: "{a} <br/>{c} {b}"
    },
    toolbox: { show : false, },
    series : [
        // speedo
        {
            name:'speedometer',
            type:'gauge',
            min:0,
            max:130,
            splitNumber:13,
            axisLine: { lineStyle: { color: [[0.7, '#1e90ff'],[0.8, '#ff4500']], width: 3, shadowColor : '#fff', shadowBlur: 1 } },
            axisLabel: { textStyle: { fontWeight: 'bolder', color: '#fff', shadowColor : '#fff', shadowBlur: 1 } },
            axisTick: { length :15, lineStyle: { color: 'auto', shadowColor : '#fff', shadowBlur: 1 } },
            splitLine: { length :25, lineStyle: { width:3, color: '#fff', shadowColor : '#fff', shadowBlur: 3 } }, // splitters
            pointer: { shadowColor : '#fff', shadowBlur: 1 },
            title : { textStyle: { fontWeight: 'bolder', fontSize: 20, fontStyle: 'italic', color: '#fff', shadowColor : '#fff', shadowBlur: 1 } },
            detail : { borderWidth: 1, borderColor: '#fff', shadowColor : '#fff', shadowBlur: 3, offsetCenter: [10, '50%'], textStyle: { fontWeight: 'bolder', color: '#fff' } },
            data:[{value: 40, name: 'km/h'}]
        },
        // rpms
        {
            name:'rpms',
            type:'gauge',
            center : ['25%', '55%'],
            radius : '50%',
            min:0,
            max:3100,
            endAngle:45,
            splitNumber:3,
            axisLine: { lineStyle: { color: [[0.29, 'lime'],[0.86, '#1e90ff'],[1, '#ff4500']], width: 2, shadowColor : '#fff', shadowBlur: 1 } },// line colors
            //axisLabel: { textStyle: { fontWeight: 'bolder', color: '#fff', shadowColor : '#fff', shadowBlur: 1 } },
            axisLabel: {
                textStyle: { fontWeight: 'bolder', color: '#fff', shadowColor : '#fff', shadowBlur: 1 },
                formatter:function(v){
                    if (v>=0 && v<=900) return '0';
                    if (v>=900 && v<=1900) return '1';
                    if (v>=1900 && v<=2900) return '2';
                    return '3';
                }
            },
            axisTick: { length :12, lineStyle: { color: 'auto', shadowColor : '#fff', shadowBlur: 1 } }, // color lines
            splitLine: { length :20, lineStyle: { width:3, color: '#fff', shadowColor : '#fff', shadowBlur: 3 } }, // splitters
            pointer: { width:5, shadowColor : '#fff', shadowBlur: 0 },
            title : { offsetCenter: [0, '-30%'], textStyle: { fontWeight: 'bolder', fontStyle: 'italic', color: '#fff', shadowColor : '#fff', shadowBlur: 1 } },
            detail : { borderColor: '#fff', shadowColor : '#fff', shadowBlur: 0, width: 80, height:30, offsetCenter: [25, '20%'], textStyle: { fontWeight: 'bolder', color: '#fff' } },
            data:[{value: 1.5, name: 'rpm'}]
        },
        // diesel
        /*
        {
            name:'diesel',
            type:'gauge',
            center : ['75%', '50%'], 
            radius : '50%',
            min:0,
            max:2,
            startAngle:135,
            endAngle:45,
            splitNumber:2,
            axisLine: { lineStyle: { color: [[0.2, 'lime'],[0.8, '#1e90ff'],[1, '#ff4500']], width: 2, shadowColor : '#fff', shadowBlur: 1 } },
            axisTick: { length :12, lineStyle: { color: 'auto', shadowColor : '#fff', shadowBlur: 1 } },
            axisLabel: {
                textStyle: { fontWeight: 'bolder', color: '#fff', shadowColor : '#fff', shadowBlur: 1 },
                formatter:function(v){
                    switch (v + '') {
                        case '0' : return 'E';
                        case '1' : return 'diesel';
                        case '2' : return 'F';
                    }
                }
            },
            splitLine: { length :15, lineStyle: { width:3, color: '#fff', shadowColor : '#fff', shadowBlur: 1 } },
            pointer: { width:2, shadowColor : '#fff', shadowBlur: 5 },
            title : { show: false },
            detail : { borderColor: '#fff', shadowColor : '#fff', shadowBlur: 0, width: 20, height:7, offsetCenter: ['50%', '-20%'], textStyle: { fontSize: 14, fontWeight: 'bold', color: '#fff' } },
            data:[{value: 0.5, name: 'gas'}]
        },
        */
       // oil temp
       {
            name:'temp',
            type:'gauge',
            center : ['75%', '50%'],    
            radius : '50%',
            min:0,
            max:120,
            startAngle:135,
            endAngle:45,
            splitNumber:2,
            axisLine: { lineStyle: { color: [[0.2, 'lime'],[0.8, '#1e90ff'],[1, '#ff4500']], width: 2, shadowColor : '#fff', shadowBlur: 1 } },
            axisTick: { show: false },
            axisLabel: {
                textStyle: { fontWeight: 'bolder', color: '#fff', shadowColor : '#fff', shadowBlur: 1 },
                formatter:function(v){
                    switch (v + '') {
                        case '0' : return 'C';
                        case '60' : return 'Oil °';
                        case '120' : return 'H';
                    }
                }
            },
            splitLine: { length :15, lineStyle: { width:3, color: '#fff', shadowColor : '#fff', shadowBlur: 1 } },
            pointer: { width:2, shadowColor : '#fff', shadowBlur: 0 },
            title : { show: false },
            detail : { borderColor: '#fff', shadowColor : '#fff', shadowBlur: 0, width: 20, height:7, offsetCenter: ['70%', '-40%'], textStyle: { fontSize: 14, fontWeight: 'bold', color: '#fff' } },
            data:[{value: 0, name: 'temp'}]
        },
        // water temp
        {
            name:'temp',
            type:'gauge',
            center : ['75%', '50%'],    
            radius : '50%',
            min:0,
            max:120,
            startAngle:315,
            endAngle:225,
            splitNumber:2,
            axisLine: { lineStyle: { color: [[0.2, '#ff4500'],[0.8, '#1e90ff'],[1, 'lime']], width: 2, shadowColor : '#fff', shadowBlur: 1 } },
            axisTick: { show: false },
            axisLabel: {
                textStyle: { fontWeight: 'bolder', color: '#fff', shadowColor : '#fff', shadowBlur: 1 },
                formatter:function(v){
                    switch (v + '') {
                        case '0' : return 'H';
                        case '60' : return 'Water °';
                        case '120' : return 'C';
                    }
                }
            },
            splitLine: { length :15, lineStyle: { width:3, color: '#fff', shadowColor : '#fff', shadowBlur: 1 } },
            pointer: { width:2, shadowColor : '#fff', shadowBlur: 0 },
            title : { show: false },
            detail : { borderColor: '#fff', shadowColor : '#fff', shadowBlur: 0, width: 20, height:7, offsetCenter: ['70%', '40%'], textStyle: { fontSize: 14, fontWeight: 'bold', color: '#fff' } },
            data:[{value: 0, name: 'temp'}]
        }
    ]
};

var myChart = echarts.init(document.getElementById('main')); 
/*
var timeTicket = setInterval(function (){
    option.series[0].data[0].value = Math.round((Math.random()*100).toFixed(2) - 0);
    option.series[1].data[0].value = (Math.random()*7).toFixed(2) - 0;
    option.series[2].data[0].value = (Math.random()*2).toFixed(2) - 0;
    option.series[3].data[0].value = (Math.random()*2).toFixed(2) - 0;
    myChart.setOption(option,true);
},2000)
*/
const socket = io('http://'+location.host);

var rpmUpping = true;
socket.on('ecuData', function (data) {
    if (data.EGT) option.series[2].data[0].value = parseInt(data.EGT);
    if (data.RPMs) option.series[1].data[0].value = parseInt(data.RPMs);
    /*
    option.series[0].data[0].value = data.kph;
    option.series[1].data[0].value = data.rpm;
    data.oilTemp ;
    */
    option.series[3].data[0].value = 120 - 1 ;
    myChart.setOption(option,true);
    console.log(data);
});
