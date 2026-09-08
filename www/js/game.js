// Function responsible to listen to wasd / arrows and send the respective json to the server SSE
// document.addEventListener('keydown', (e) => {
// 	let dx = 0;
// 	let dy = 0;

// 	if (e.key == 'ArrowUp')
// 		dy = -5;
// 	if (e.key == 'ArrowDown')
// 		dy = 5;
// 	if (e.key == 'ArrowLeft')
// 		dx = -5;
// 	if (e.key == 'ArrowRight')
// 		dx = 5;

// 	if (dx == 0 && dy == 0)
// 		return;
// 	e.preventDefault();

// 	fetch('game/move', {
// 		method: 'POST',
// 		body: JSON.stringify({ dx, dy })
// 	});
// });

// function readState() {
// 	const gameState = JSON.parse(text);
//
// if (gameState.type === "move")
// {
// 		let penguin = docum(gameState.id);
//			penguin.style.x = g
//			penguin.style.y = g
//			direction/sprite = g
//			direction/sprite = g
// }
//
// if (JOIN) {
//     how to add penguin
// }
// else if (LEFT) {
// 		how to remove penguin
// }

function startGame() {

}

function getCookie(name) {
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) return decodeURIComponent(parts.pop().split(';').shift());
    return null;
}

function setCookie(name, value, days = 30) {
    const maxAge = days * 24 * 60 * 60;
    document.cookie = `${name}=${value}; Path=/; Max-Age=${maxAge}`;
}

function htmlToElement(html) {
    const template = document.createElement('template');
    html = html.trim();
    template.innerHTML = html;
    return template.content.firstChild;
}

const username = getCookie("cp_session") || "guest";

const safeUserKey = username.replace(/\s+/g, '_');

function getUserPosition(userKey) {
    const cookieKey = `cp_pos_${userKey}`;
    const savedPos = getCookie(cookieKey);

    if (savedPos) {
        const [x, y] = savedPos.split(",").map(Number);
        if (!isNaN(x) && !isNaN(y)) {
            return { x, y };
        }
    }
    return { x: 100, y: 100 };
}

const initialPos = getUserPosition(safeUserKey);

const player = {
    username: username,
    x: initialPos.x,
    y: initialPos.y
};

const arena = document.getElementById("arena");

function renderState(state) {
    if (!Array.isArray(state)) return;

    let activeIds = [];

    state.forEach(({ username, x, y }) => {
        if (!username) return;

        const safeId = `penguin-${username.replace(/\s+/g, '_')}`;
        activeIds.push(safeId);

        let el = document.getElementById(safeId);
        if (!el) {
            el = htmlToElement(`<div class="penguin" id="${safeId}">🐧<p>${username}</p></div>`);
            arena.append(el);
        }
        el.style.top = `${y}px`;
        el.style.left = `${x}px`;
    });

    document.querySelectorAll('.penguin').forEach(el => {
        if (!activeIds.includes(el.id)) {
            el.remove();
        }
    });
}

async function update(pData) {
    try {
        const res = await fetch("/update", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(pData)
        });
        const state = await res.json();
        renderState(state);
    } catch (e) {}
}

setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
update(player);

setInterval(() => {
    update(player);
}, 1000);

document.addEventListener("keyup", (e) => {
    const speed = 30;
    let moved = false;

    if (e.key === "ArrowUp") { player.y = Math.max(0, player.y - speed); moved = true; }
    if (e.key === "ArrowDown") { player.y = Math.min(260, player.y + speed); moved = true; }
    if (e.key === "ArrowLeft") { player.x = Math.max(0, player.x - speed); moved = true; }
    if (e.key === "ArrowRight") { player.x = Math.min(460, player.x + speed); moved = true; }

    if (moved) {
        setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
        update(player);
    }
});

document.addEventListener("DOMContentLoaded", () => {
    const logoutBtn = document.getElementById("logoutButton");
    if (logoutBtn) {
        logoutBtn.addEventListener("click", async () => {
            setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
            try {
                await fetch("/update", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ username: player.username })
                });
            } catch (e) {}
            window.location.href = "/cgi-bin/logout.py";
        });
    }
});

window.addEventListener("pagehide", () => {
    setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
    const data = JSON.stringify({ username: player.username });
    if (navigator.sendBeacon) {
        navigator.sendBeacon("/update", data);
    }
});
