import { SerialPort } from 'serialport';
import fs from 'fs';

const potential = fs
    .readdirSync('/dev')
    .filter((id) => id.includes('usbserial'));

const port = new SerialPort({ path: `/dev/${potential[0]}`, baudRate: 9600 });

const send = (d) =>
    port.write((typeof d === 'string' ? d : JSON.stringify(d)) + '\n');

// const steps = [
//     [100, 100],
//     [200, -100],
//     [100, 200],
//     [0, 0],
//     [100, 100],
// ];

const pixels = [
    // [0.8, 0],
    [0.8, 0.1],
    [0.7, 0.1],
    [0.7, 0],
    [0.8, 0],
    // [0.9, 0],
];

const sendNext = () => {
    if (pixels.length) {
        const [x, y] = pixels.shift();
        console.log('next', x, y);
        send({ cmd: 2, x, y });
    }
    // if (steps.length) {
    //     const [arm1, arm2] = steps.shift();
    //     console.log('next', arm1, arm2);
    //     send({ cmd: 1, arm1, arm2 });
    // }
};

const handle = (line) => {
    console.log(JSON.stringify(line));
    if (line === 'Done') {
        // sendNext();
    }
};

port.open(() => {
    let buf = '';
    port.on('data', (data) => {
        buf += data.toString('utf8');
        if (buf.includes('\n')) {
            const lines = buf.split(/\r?\n/);
            buf = lines.pop();
            lines.forEach(handle);
        }
    });
    console.log('connected');
    send('ping');
    // sendNext();
    send({ cmd: 1, arm1: 0, arm2: 0 });
    // send({ cmd: 2, x: 1, y: 0 });
    // pixels.forEach(([x, y]) => {
    //     send({ cmd: 2, x, y, sim: true });
    // });
    // send({ cmd: 3 });
});
