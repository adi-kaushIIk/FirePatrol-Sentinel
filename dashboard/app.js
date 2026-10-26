const STORE_KEY='fps_dashboard_state_v1';
const DEFAULT_STATE={running:false,distance:85,mq:420,flame:false,speed:0,logs:[]};
function loadState(){try{return {...DEFAULT_STATE,...JSON.parse(localStorage.getItem(STORE_KEY)||'{}')}}catch(e){return {...DEFAULT_STATE}}}
function saveState(s){localStorage.setItem(STORE_KEY,JSON.stringify(s));window.dispatchEvent(new Event('fps-state-change'))}
function addLog(title,detail,level='info'){const s=loadState();s.logs.unshift({time:new Date().toISOString(),title,detail,level});s.logs=s.logs.slice(0,250);saveState(s)}
function hazard(s){if(s.flame)return 'fire';if(s.mq>=1800)return 'smoke';return 'none'}
function fmtTime(iso){return new Date(iso).toLocaleString([], {month:'short',day:'2-digit',hour:'2-digit',minute:'2-digit',second:'2-digit'})}
function setState(patch){const s={...loadState(),...patch};saveState(s);return s}
function initDashboard(){
 let history=Array.from({length:32},(_,i)=>85+Math.round(Math.sin(i*.45)*8));
 const update=()=>{const s=loadState(),h=hazard(s),blocked=s.distance<=30;
 document.getElementById('patrolText').textContent=h!=='none'?'Emergency stop':s.running?'Patrolling':'Standby';
 document.getElementById('distanceVal').textContent=s.distance;document.getElementById('distanceBar').style.width=Math.min(100,s.distance/200*100)+'%';
 document.getElementById('mqVal').innerHTML=s.mq+'<small> / 4095</small>';document.getElementById('mqBar').style.width=(s.mq/4095*100)+'%';
 document.getElementById('flameVal').textContent=s.flame?'DETECTED':'Clear';document.getElementById('flameDot').className='dot '+(s.flame?'bad':'good');document.getElementById('flameNote').textContent=s.flame?'Flame input active':'No flame detected';
 const motor=s.running&&h==='none'?5:0;document.getElementById('speedVal').innerHTML=motor+'<small>% PWM</small>';document.getElementById('speedBar').style.width=motor+'%';
 document.getElementById('telemetryStatus').textContent=s.running?'Patrol active':'Idle';
 const badge=document.getElementById('hazardBadge');badge.className='chip '+(h==='none'?'good':'warn');badge.textContent=h==='fire'?'Fire detected':h==='smoke'?'Smoke detected':'All clear';
 document.getElementById('obstacleState').textContent=blocked?'Obstacle at '+s.distance+' cm · scan and turn logic would engage':'Path clear · '+(s.running?'forward patrol active':'forward patrol ready');document.getElementById('obstacleTag').textContent=blocked?'BLOCKED':'READY';
 document.getElementById('smokeState').textContent=s.mq>=1800?'MQ-2 reading above threshold':'Reading below threshold';document.getElementById('smokeTag').textContent=s.mq>=1800?'ALERT':'CLEAR';
 document.getElementById('alarmState').textContent=h==='none'?'Buzzer inactive':h==='fire'?'Fire alarm pattern active':'Smoke alarm pattern active';document.getElementById('alarmTag').textContent=h==='none'?'OFF':'ON';
 if(h!=='none'){document.getElementById('speedVal').innerHTML='0<small>% PWM</small>';document.getElementById('speedBar').style.width='0%'}
 history.push(s.distance);history=history.slice(-32);const pts=history.map((v,i)=>`${i*700/(history.length-1)},${200-Math.max(0,Math.min(200,v))/200*185}`).join(' ');document.getElementById('linePath').setAttribute('d','M'+pts.replaceAll(' ',',L'));document.getElementById('areaPath').setAttribute('d','M0,205 L'+pts.replaceAll(' ',',L')+' L700,205 Z');
 };
 document.getElementById('startBtn').onclick=()=>{let s=loadState();if(hazard(s)!=='none'){addLog('Patrol start blocked','Clear the simulated hazard before restarting.','warning');return}setState({running:true,speed:5});addLog('Patrol started','Simulation set to 5% PWM target.');update()};
 document.getElementById('stopBtn').onclick=()=>{setState({running:false,speed:0});addLog('Rover stopped','Manual stop command in dashboard simulation.','warning');update()};
 document.getElementById('obstacleBtn').onclick=()=>{setState({distance:18});addLog('Obstacle detected','Simulated front distance is 18 cm.','warning');update()};
 document.getElementById('smokeBtn').onclick=()=>{setState({mq:2300,flame:false,running:false,speed:0});addLog('Smoke threshold exceeded','MQ-2 simulated reading rose above 1800.','warning');update()};
 document.getElementById('fireBtn').onclick=()=>{setState({flame:true,running:false,speed:0});addLog('Fire detected','Simulated active-LOW flame input. Rover stopped and alarm activated.','critical');update()};
 document.getElementById('clearHazardBtn').onclick=()=>{setState({distance:85,mq:420,flame:false});addLog('Hazards cleared','Simulated sensor readings returned to normal.');update()};
 document.getElementById('clearBtn').onclick=()=>{setState({...DEFAULT_STATE,logs:loadState().logs});addLog('Simulation reset','Sensor values reset to normal and patrol stopped.');update()};
 window.addEventListener('fps-state-change',update);window.addEventListener('storage',update);update();
 // Make the simulation feel alive only while patrol is active and no hazard is present.
 setInterval(()=>{const s=loadState();if(s.running&&hazard(s)==='none'){const next=Math.max(35,Math.min(150,s.distance+Math.round(Math.random()*12-6)));setState({distance:next});}},1800);
}
function initSensors(){
 const d=document.getElementById('distanceInput'),m=document.getElementById('mqInput'),f=document.getElementById('flameInput');
 const update=()=>{const s=loadState();d.value=s.distance;m.value=s.mq;f.checked=s.flame;document.getElementById('distanceRead').textContent=s.distance;document.getElementById('mqRead').textContent=s.mq;document.getElementById('flameRead').textContent=s.flame?'Flame detected':'No flame';
 const blocked=s.distance<=30,smoke=s.mq>=1800;
 const dc=document.getElementById('distState');dc.className='chip '+(blocked?'warn':'good');dc.textContent=blocked?'OBSTACLE':'CLEAR';
 const mc=document.getElementById('smokeStateChip');mc.className='chip '+(smoke?'warn':'good');mc.textContent=smoke?'SMOKE ALERT':'CLEAR';
 const fc=document.getElementById('flameStateChip');fc.className='chip '+(s.flame?'warn':'good');fc.textContent=s.flame?'FIRE ALERT':'CLEAR';};
 d.oninput=()=>{const old=loadState().distance,v=+d.value;setState({distance:v});document.getElementById('distanceRead').textContent=v;if((old>30&&v<=30)||(old<=30&&v>30))addLog(v<=30?'Obstacle detected':'Obstacle cleared',`Distance changed to ${v} cm.`,v<=30?'warning':'info');update()};
 m.oninput=()=>{const old=loadState().mq,v=+m.value;setState({mq:v});document.getElementById('mqRead').textContent=v;if(old<1800&&v>=1800)addLog('Smoke threshold exceeded',`MQ-2 reading changed to ${v}.`,'warning');if(old>=1800&&v<1800)addLog('Smoke reading normal',`MQ-2 reading returned to ${v}.`,'info');update()};
 f.onchange=()=>{setState({flame:f.checked,running:f.checked?false:loadState().running,speed:f.checked?0:loadState().speed});addLog(f.checked?'Fire detected':'Flame alert cleared',f.checked?'Simulated flame input activated.':'Simulated flame input returned to clear. ',f.checked?'critical':'info');update()};
 document.getElementById('resetSensors').onclick=()=>{setState({distance:85,mq:420,flame:false});addLog('Sensor values reset','Sensor Lab values returned to defaults.');update()};
 window.addEventListener('fps-state-change',update);window.addEventListener('storage',update);update();
}
function initLogs(){
 const list=document.getElementById('logList'),filter=document.getElementById('filterSelect');
 const update=()=>{const logs=loadState().logs,level=filter.value,filtered=logs.filter(l=>level==='all'||l.level===level);document.getElementById('totalEvents').textContent=logs.length;document.getElementById('hazardEvents').textContent=logs.filter(l=>l.level==='warning'||l.level==='critical').length;document.getElementById('lastEvent').textContent=logs.length?logs[0].title:'None yet';list.innerHTML='';document.getElementById('emptyLogs').hidden=filtered.length>0;
 filtered.forEach(l=>{const row=document.createElement('div');row.className='log-row';const time=document.createElement('div');time.className='log-time';time.textContent=fmtTime(l.time);const marker=document.createElement('span');marker.className='log-marker '+l.level;const body=document.createElement('div');body.className='log-body';const b=document.createElement('b');b.textContent=l.title;const p=document.createElement('p');p.textContent=l.detail;body.append(b,p);const tag=document.createElement('span');tag.className='log-level '+l.level;tag.textContent=l.level.toUpperCase();row.append(time,marker,body,tag);list.append(row)});
 };
 filter.onchange=update;
 document.getElementById('addEventBtn').onclick=()=>{const titles=[['Patrol checkpoint','Rover reached a simulated checkpoint.','info'],['Obstacle detected','Simulated distance fell below 30 cm.','warning'],['Sensor health check','All simulated inputs are responding.','info']];const e=titles[Math.floor(Math.random()*titles.length)];addLog(...e);update()};
 document.getElementById('clearLogs').onclick=()=>{if(confirm('Clear all browser-stored prototype logs?')){setState({logs:[]});update()}};
 document.getElementById('exportBtn').onclick=()=>{const logs=loadState().logs;const csv=[['timestamp','level','event','details'],...logs.map(l=>[l.time,l.level,l.title,l.detail])].map(row=>row.map(v=>'"'+String(v).replaceAll('"','""')+'"').join(',')).join('\r\n');const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([csv],{type:'text/csv'}));a.download='firepatrol-logs.csv';a.click();URL.revokeObjectURL(a.href)};
 window.addEventListener('fps-state-change',update);window.addEventListener('storage',update);update();
}
