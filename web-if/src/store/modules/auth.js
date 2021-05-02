import {
    AUTH_REQUEST,
    AUTH_ERROR,
    AUTH_SUCCESS,
    AUTH_LOGOUT,
  } from "../actions/auth";
  import axios from 'axios';

    const state = {
      current_user: localStorage.getItem("wwv-webcam.user") || "",
      current_password: localStorage.getItem("wwv-webcam.pass") || "",
      status: "",
      errorMessage: "",
      hasLoadedOnce: false
    };

    const getters = {
      isAuthenticated: state => (state.current_user != ""),
      authStatus: state => state.status,
      currentUser: state => state.current_user,
      currentPassword: state => state.current_password
    };

    const actions = {
      [AUTH_REQUEST]: ({ commit }, user) => {
        return new Promise((resolve, reject) => {
          commit(AUTH_REQUEST);
          axios.get('https://wwv-schwarzwald.de/webcam/api/authenticate?user='+user.user+'&password='+user.password)
            .then(resp => {
              if (resp.data.authenticated) {
                localStorage.setItem("wwv-webcam.user", user.user);
                localStorage.setItem("wwv-webcam.pass", user.password);
                commit(AUTH_SUCCESS, user);
                resolve(resp);
              } else {
                commit(AUTH_ERROR, resp);
                localStorage.removeItem("wwv-webcam.user");
                localStorage.removeItem("wwv-webcam.pass");
                reject(resp);
              }
            })
        });
      },
      [AUTH_LOGOUT]: ({ commit }) => {
        return new Promise(resolve => {
          commit(AUTH_LOGOUT);
          localStorage.removeItem("wwv-webcam.user");
          localStorage.removeItem("wwv-webcam.pass");
          resolve();
        });
      }
    };

    const mutations = {
      [AUTH_REQUEST]: state => {
        state.status = "loading";
      },
      [AUTH_SUCCESS]: (state, user) => {
        state.status = "success";
        state.errorMessage = "";
        state.hasLoadedOnce = true;
        state.current_user = user.user
        state.current_password = user.password
      },
      [AUTH_ERROR]: (state, resp) => {
        state.status = "error";
        if (!resp.data.authenticated) {
          state.errorMessage = "Error: Unknown user or wrong password"
        } else {
          state.errorMessage = "Communication error. Please try again Later."
        }
        state.hasLoadedOnce = true;
      },
      [AUTH_LOGOUT]: (state, msg) => {
        state.errorMessage = (msg) ? msg : ""
        state.current_user = ""
        state.current_password = ""
      }
    };

    export default {
      state,
      getters,
      actions,
      mutations
    };
