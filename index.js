"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.awaitSync = exports.deasync = void 0;
/// <reference types="./missing-types" />
const binding_node_1 = require("./build/Release/binding.node");
function loopWhile(pred) {
    while (pred()) {
        process._tickCallback();
        if (pred())
            (0, binding_node_1.run)();
    }
}
/**
 * Determine whether the value is a Promise.
 *
 * @see https://promisesaplus.com/
 */
function isThenable(value) {
    return typeof value.then === "function";
}
// Can't use enum as async-to-sync breaks the control flow analyzing.
const Pending = 0;
const Fulfilled = 1;
const Rejected = 2;
/**
 * Generic wrapper of async function with conventional API signature
 * `function (...args, (error, result) => {})`.
 *
 * Returns `result` and throws `error` as exception if not null.
 *
 * @param fn the original callback style function
 * @return the wrapped function
 */
function deasync(fn) {
    return function (...args) {
        let state = Pending;
        let resultOrError;
        args.push((err, res) => {
            if (err) {
                resultOrError = err;
                state = Rejected;
            }
            else {
                resultOrError = res;
                state = Fulfilled;
            }
        });
        fn.apply(this, args);
        loopWhile(() => state === Pending);
        if (state === Rejected) {
            throw resultOrError;
        }
        else {
            return resultOrError;
        }
    };
}
exports.deasync = deasync;
/**
 * Similar with the keyword `await` but synchronously.
 *
 * @param promise A Promise or any value to wait for
 * @return Returns the fulfilled value of the promise, or the value itself if it's not a Promise.
 */
function awaitSync(promise) {
    let state = Pending;
    let resultOrError;
    if (!isThenable(promise)) {
        return promise;
    }
    promise.then(res => {
        resultOrError = res;
        state = Fulfilled;
    }, err => {
        resultOrError = err;
        state = Rejected;
    });
    loopWhile(() => state === Pending);
    if (state === Rejected) {
        throw resultOrError;
    }
    else {
        return resultOrError;
    }
}
exports.awaitSync = awaitSync;
//# sourceMappingURL=index.js.map