const canvas = document.getElementById('gameCanvas');
const ctx = canvas.getContext('2d');

let localPlayerId = null;
let players = {};

const eventSource = new EventSource('/game/stream');

eventSource.onopen = () => console.log('SSE Connected');
eventSource.onerror = () => console.error('SSE error');

evtSource.onmessage = (event) => {
    const msg = JSON.parse(event.data);
    if (msg.type === 'init') {
        players = {};
        msg.players.forEach(p => { players[p.id] = p; });
        localPlayerId = msg.you;   // the server tells us which one we are
    } else if (msg.type === 'join') {
        players[msg.player.id] = msg.player;
    } else if (msg.type === 'leave') {
        delete players[msg.id];
    } else if (msg.type === 'move') {
        if (players[msg.id]) {
            players[msg.id].x = msg.x;
            players[msg.id].y = msg.y;
        }
    }
    draw();
};

// ---------- Movement ----------
document.addEventListener('keydown', (e) => {
    let dx = 0, dy = 0;
    if (e.key === 'ArrowUp')    dy = -5;
    if (e.key === 'ArrowDown')  dy = 5;
    if (e.key === 'ArrowLeft')  dx = -5;
    if (e.key === 'ArrowRight') dx = 5;
    if (dx === 0 && dy === 0) return;
    e.preventDefault();

    fetch('/game/move', {
        method: 'POST',
        body: JSON.stringify({ dx, dy })
    });
});

// ---------- Drawing ----------
function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    for (const id in players) {
        const p = players[id];
        ctx.fillStyle = (id == localPlayerId) ? '#ff6600' : '#336699';
        ctx.beginPath();
        ctx.arc(p.x, p.y, 15, 0, Math.PI * 2);
        ctx.fill();
        ctx.fillStyle = '#fff';
        ctx.font = '12px sans-serif';
        ctx.fillText(p.name, p.x - 15, p.y - 20);
    }
}
