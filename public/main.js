//SETTINGS
//////////
const dafaultSettings = {
    circlesNumber: 64,
    sizeMin: 2,
    sizeMax: 16,
    sizeGlowing: 16,
    sizeGlowingVelocity: 1,
    hueMin: 64,
    hueMax: 128,
    hueGlowing: 64,
    hueGlowingVelocity: 6,
    alphaMin: 30,
    alphaMax: 80,
    velocityInitialMin: 0,
    velocityInitialMax: 2,
    velocityFinalMax: 10,
    accelerationMin: 1,
    accelerationMax: 3,
    accelerationDivider: 50,
}
const settings = {
    showTrails: false,
    isRunning: false
};
renderUISettings();

//LOGIC
///////
const canvas = document.getElementById('canvas');
const context = canvas.getContext('2d');
let animationFrameId;

const setCanvasSizing = () => {
    canvas.setAttribute('width', canvas.clientWidth);
    settings.canvasWidth = canvas.clientWidth;
    canvas.setAttribute('height', canvas.clientHeight);
    settings.canvasHeight = canvas.clientHeight;
}

function renderUISettings() {
    const wrapper = document.getElementById('inputs');
    let html = "";
    Object.entries(dafaultSettings).forEach(([k, val]) => {
        html += `<label for="${k}">${k}: 
            <input type="number" value="${val}" id="${k}" min="0" />
        </label>`;
    })
    wrapper.innerHTML = html;
}

function readUISettings() {
    Object.entries(dafaultSettings).forEach(([k, val]) => {
        const updatedVal = document.getElementById(k).value;
        settings[k] = Number.isInteger(Number.parseInt(updatedVal)) && updatedVal >= 0 ? updatedVal : dafaultSettings[k];
    });
}

const drawCircle = (x, y, radious, r, g, b, a) => {
    context.beginPath();
    context.arc(x, y, radious, 0, Math.PI*2, false);
    context.fillStyle = `rgba(${r},${g},${b},${a/100})`;
    context.fill();
}

const render = () => {
    
    if (settings.showTrails) {
        context.fillStyle = 'rgba(255, 255, 255, .075)';
        context.fillRect(0, 0, canvas.width, canvas.height);
    } else {
        context.clearRect(0, 0, settings.canvasWidth, settings.canvasHeight);
    }

    //Call WebAssembly to fast-compute the new circles data
    _updateData();

    //Here we divide by 4 because we receive the pointer and size in bytes, but we work with integers of 4 bytes each
    const itemSize = _getItemSizeInBytes() / 4;
    const memoryStartingLocation = _getCirclesMemoryPointerInBytes() / 4;
    //We use HEAP32 that already groups and represent for us the data in integers of 4 bytes each
    const circlesData = Module.HEAP32.subarray(memoryStartingLocation, memoryStartingLocation + (itemSize * settings.circlesNumber));

    for (let i=0; i<circlesData.length; i+=itemSize ) {
        let circle = circlesData.slice(i, i+itemSize);
        drawCircle(...circle);
    };

    animationFrameId = requestAnimationFrame(render);
}

const fullApplicationPipeline = () => {
    cancelAnimationFrame(animationFrameId);
    readUISettings();
    setCanvasSizing();
    _initSettings();
    _generateData();
    console.log("JS> start rendering...");
    enableButtons(true);
    render();
}

const collapseSettings = () => {
    const style = document.getElementById('inputs').style;
    const newStile = style.display === 'none' ? 'grid' : 'none';
    style.display = newStile;

    const newText = newStile === 'none' ? 'expand' : 'collapse';
    document.getElementById('collapseSettings').innerText = newText;

    fullApplicationPipeline();
}

const enableButtons = (isRunning) => {
    document.getElementById('pauseBtn').disabled = !isRunning;
    document.getElementById('resumeBtn').disabled = isRunning;
}

Module['onRuntimeInitialized'] = () => {
    fullApplicationPipeline();
    window.addEventListener('resize', fullApplicationPipeline);
    window.updateSettings = fullApplicationPipeline;

    window.pause = () => {
        cancelAnimationFrame(animationFrameId);
        enableButtons(false);
    };

    window.resume = () => {
        enableButtons(true);
        render();
    }
}