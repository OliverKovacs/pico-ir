const fs = require("node:fs");
const readline = require("node:readline");

const FRAME_LEN = {
    nec:  32,   // 16 bit addr + 16 bit cmd
    sony: 12,   // 7 bit cmd + 5 bit addr
};

const HEADER_ON = {
    nec:  [ 80_000, 120_000 ],  // 9 ms
    sony: [ 25_000,  35_000 ],  // 2.4 ms
};

const HEADER_OFF = {
    nec:  [ 40_000, 60_000 ],   // 4.5 ms
    sony: [  4_000,  8_000 ],   // 0.6 ms
};

const DATA_LONG = {
    nec:  [ 10_000, 20_000 ],   // 1.6875 ms
    sony: [ 10_000, 20_000 ],   // 1.2 ms
};

const DATA_SHORT = {
    nec:  [ 4_000, 10_000 ],    // 0.5625 ms
    sony: [ 4_000, 10_000 ],    // 0.6 ms
};

// nec or sony
const protocol = process.argv[2];
if (protocol !== "nec" && protocol !== "sony") throw new Error("invalid protocol");

fs.writeFileSync("./data.raw", "");
fs.writeFileSync("./frames.bin", "");
fs.writeFileSync("./frames.hex", "");
fs.writeFileSync("./history.txt", "");

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    terminal: false,
});

const BIN_TO_HEX = Object.fromEntries(Array
    .from({ length: 16 }, (_, i) => i)
    .map(e => [ e.toString(2).padStart(4, "0"), e.toString(16) ]));

const frame_to_hex = (message: string) => message
    .match(/[01]{4}/g)!
    .map(e => BIN_TO_HEX[e])
    .join("");

let value = true;

rl.on("line", (line: string) => {
    if (line === "") return;
    const buffer = line.slice(0, -1).split(",").map(e => +e);
    fs.appendFileSync("./data.raw", line);
    for (length of buffer) {
        edge(length, value);
        value = !value;
    }
});

const _frames: string[] = [];
const _history: number[] = [];
let frame = "";
let state = "init";

const add_frame = (frame: string) => {
    console.log(frame);
    const index = _frames.indexOf(frame);
    if (index !== -1) {
        _history.push(index);
        return;
    }
    _frames.push(frame);
    _history.push(frames.length - 1);
    fs.writeFileSync("./frames.bin", _frames.join("\n") + "\n", "utf-8");
    fs.writeFileSync("./frames.hex", _frames.map(frame_to_hex).join("\n") + "\n", "utf-8");
    fs.writeFileSync("./history.txt", _history.join(",") + "\n", "utf-8");
};

const in_range = (value: number, [ min, max ]: number[]) => min <= value && value <= max;

const edge = (length: number, value: boolean) => {
    console.log(state, length, value, frame, frame.length)
    switch(state) {
        case "init":
            frame = "";
            if (value && in_range(length, HEADER_ON[protocol])) {
                state = "header";
            }
            break;
        case "header":
            state = !value && in_range(length, HEADER_OFF[protocol])
                ? "message"
                : "init";
            break;
        case "message":
            if (protocol === "nec") {
                parse_nec(length, value);
            }
            else {
                parse_sony(length, value);
            }
    }
};

const parse_nec = (length: number, value: boolean) => {
    if (value) {
        // on
        if (in_range(length, DATA_SHORT["nec"])) return;
        state = "init";
    }
    else {
        // off
        if (in_range(length, DATA_SHORT["nec"])) {
            frame += "0";
            if (frame.length === FRAME_LEN["nec"]) add_frame(frame);
        }
        else if (in_range(length, DATA_LONG["nec"])) {
            frame += "1";
            if (frame.length === FRAME_LEN["nec"]) add_frame(frame);
        }
    }
};

const parse_sony = (length: number, value: boolean) => {
    if (value) {
        // on
        if (in_range(length, DATA_SHORT["sony"])) {
            frame += "0";
            if (frame.length === FRAME_LEN["sony"]) add_frame(frame);
        }
        else if (in_range(length, DATA_LONG["sony"])) {
            frame += "1";
            if (frame.length === FRAME_LEN["sony"]) add_frame(frame);
        }
    }
    else {
        // off
        if (!in_range(length, DATA_SHORT["sony"])) state = "init";
    }
};
