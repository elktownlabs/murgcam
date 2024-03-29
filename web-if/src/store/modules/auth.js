import {
    AUTH_REQUEST,
    AUTH_ERROR,
    AUTH_SUCCESS,
    AUTH_LOGOUT,
  } from "../actions/auth";
  import axios from 'axios';

    const state = {
      current_user: localStorage.getItem("wwv-webcam.user") || "",
      current_user_fullname: localStorage.getItem("wwv-webcam.fullname") || "",
      current_user_avatar: localStorage.getItem("wwv-webcam.avatar") || null,
      current_user_initials: localStorage.getItem("wwv-webcam.initials") || "",
      current_token: localStorage.getItem("wwv-webcam.token") || "",
      current_user_rights: JSON.parse(localStorage.getItem("wwv-webcam.rights")) || [],
      status: "",
      errorMessage: "",
      hasLoadedOnce: false
    };

    const getters = {
      isAuthenticated: state => (state.current_user != ""),
      authStatus: state => state.status,
      currentToken: state => state.current_token,
      currentUser: state => state.current_user,
      currentUserFullName: state => state.current_user_fullname,
      currentUserAvatar: state => state.current_user_avatar,
      currentUserInitials: state => state.current_user_initials,
      hasRight: state => right => { return state.current_user_rights.includes(right) }
    };

    const actions = {
      [AUTH_REQUEST]: ({ commit }, user) => {
        return new Promise((resolve, reject) => {
          commit(AUTH_REQUEST);
          axios.post(process.env['VUE_APP_BACKENDURL']+'/authenticate',  {user: user.user, password: user.password})
            .then(resp => {
              if (resp.data.authenticated) {
                localStorage.setItem("wwv-webcam.token", resp.data.token);
                localStorage.setItem("wwv-webcam.user", resp.data.user);
                localStorage.setItem("wwv-webcam.fullname", resp.data.name);
                localStorage.setItem("wwv-webcam.initials", resp.data.initials);
                localStorage.setItem("wwv-webcam.avatar", resp.data.avatar);
                localStorage.setItem("wwv-webcam.rights", JSON.stringify(resp.data.rights));
                commit(AUTH_SUCCESS, resp.data);
                resolve(resp);
              } else {
                commit(AUTH_ERROR, resp);
                localStorage.removeItem("wwv-webcam.token");
                localStorage.removeItem("wwv-webcam.user");
                localStorage.removeItem("wwv-webcam.fullname");
                localStorage.removeItem("wwv-webcam.initials");
                localStorage.removeItem("wwv-webcam.avatar");
                localStorage.removeItem("wwv-webcam.rights");
                reject(resp);
              }
            })
        });
      },
      [AUTH_LOGOUT]: ({ commit }) => {
        return new Promise(resolve => {
          commit(AUTH_LOGOUT);
          localStorage.removeItem("wwv-webcam.token");
          localStorage.removeItem("wwv-webcam.user");
          localStorage.removeItem("wwv-webcam.fullname");
          localStorage.removeItem("wwv-webcam.initials");
          localStorage.removeItem("wwv-webcam.avatar");
          localStorage.removeItem("wwv-webcam.rights");
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
        state.current_user_fullname = user.name
        state.current_user_avatar = user.avatar
        state.current_user_initials = user.initials
        state.current_user_rights = user.rights
        state.current_token = user.token
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
        state.current_user_fullname = ""
        state.current_user_avatar = ""
        state.current_user_initials = ""
        state.current_user_rights = ""
        state.current_token = ""
      }
    };

    export default {
      state,
      getters,
      actions,
      mutations
    };
